/********************************************************************
*
* Copyright (C) 2019 Fondazione Istituto Italiano di Tecnologia (IIT)
* All Rights Reserved
*
*/

/**
 * @file src/liteConv/liteConv.cpp
 * @authors: Leandro de Souza Rosa <leandro.desouzarosa@iit.it>
 */

#include "liteConv.h"

void UpdateAndConvolve::initialise(
        cv::Mat &m_sae,
        cv::Mat &m_img,
        cv::Mat &convolved_img,
        cv::Mat &m_kernel,
        double m_alpha,
        double *last_ts,
        std::string m_name,
        unsigned int m_height,
        unsigned int m_width,
        bool *m
        #if LOG==1
            , std::vector<std::tuple<int, double, double>> *data
        #endif
)
{
    sae = m_sae;
    img = m_img;
    convolved = convolved_img;
    kernel = m_kernel;
    alpha = m_alpha;
    img_ts = last_ts; 
    name = m_name;
    mlock = m;
    #if LOG==1
        d = data;
    #endif
    
    coefs = cv::Mat(m_height, m_width, CV_64F, cv::Scalar(0));
    decays = cv::Mat(m_height, m_width, CV_64F, cv::Scalar(0));
    updated_img = cv::Mat(m_height, m_width, CV_64F, cv::Scalar(0));
    norm_img = cv::Mat(m_height, m_width, CV_64F, cv::Scalar(0));
}

void UpdateAndConvolve::run()
{
    while(!isStopping())
    {
        if(*mlock){
            #if LOG==1
                double tic = yarp::os::Time::now();
            #endif
           
            // calculate the exponent of the decay for the whole img
            coefs = alpha*(sae - *img_ts);
   
            // compute the decays = e^(-alpha*dt) - saves on "decays" mat
            cv::exp(coefs, decays);
   
            // create updated image by decaying all pixels to the last ts
            updated_img = img.mul(decays);
   
            // apply convolution
            cv::filter2D(updated_img, convolved, CV_64F, kernel);
        
            #if LOG==1
                d->push_back(std::tuple<int, double, double>(1, tic, (yarp::os::Time::now()-tic)));
            #endif
   
            #ifdef VIS
                normalize(convolved, norm_img, 0, 1, NORM_MINMAX);
                cv::imshow(name, (1-norm_img));// invert colours
                cv::waitKey(1);
            #endif
            *mlock = false;
        } // if mlock
    }// while !isStopping()
}// run()

bool LiteConv::configure(yarp::os::ResourceFinder& rf)
{
    
    // open yarp ports (in/out) associated to the module
    setName((rf.check("name", yarp::os::Value("/liteConv")).asString()).c_str());

    // Open YARP ports (in/out) associated to the module
    if(!m_inPort.open(getName()+"/AE:i"))
    {
        yError() << "Could not open input port";
        return false;
    }
    
    /* set parameters */
    m_height = static_cast<unsigned int>(rf.check("height", yarp::os::Value(480)).asInt32());
    m_width = static_cast<unsigned int>(rf.check("width", yarp::os::Value(640)).asInt32());
    m_alpha = static_cast<double>(rf.check("alpha", yarp::os::Value(1*M_PI)).asFloat64());
    m_ksize = static_cast<unsigned int>(rf.check("kSize", yarp::os::Value(3)).asInt32());
    m_sigma = static_cast<double>(rf.check("sigma", yarp::os::Value(0.3)).asFloat64());
    
    if (m_ksize%2==0 || m_ksize <= 0)
    {
        yInfo() << "kernel size must be positive (>0), odd!";
        return false;
    } 

    m_kernel = cv::getGaussianKernel(m_ksize, m_sigma);
    m_kernel = m_kernel*m_kernel.t();
   
    // intermediate image
    m_img = cv::Mat(m_height, m_width, CV_64F, cv::Scalar(0)); 
    // SAE
    m_sae = cv::Mat(m_height, m_width, CV_64F, cv::Scalar(0));

    #if LOG==0 || LOG==1 || LOG==2
        yInfo() << "Logging input port delay";
        std::string testName = rf.check("testName", yarp::os::Value("log")).asString();
    #endif
    
    #if LOG==0
        logFileName = std::string(std::getenv("HOME"))+std::string("/results/paper_convolution") + getName() + "/" + testName + std::string("_delay.txt");
        logFilePath = logFileName;
    #elif LOG ==1
        logFileName = std::string(std::getenv("HOME"))+std::string("/results/paper_convolution") + getName() + "/" + testName + std::string("_threadsTimes.txt");
        logFilePath = logFileName;
    #elif LOG ==2
        logFileName = std::string(std::getenv("HOME"))+std::string("/results/paper_convolution") + getName() + "/" + testName + std::string("_accuracy.txt");
        logFilePath = logFileName;
    #endif

    #if LOG==0 || LOG==1 || LOG==2
        //open the file
        if(!fs::exists(logFilePath.parent_path()))
        {
            fs::create_directories(logFilePath.parent_path());
        }
        log.open(logFileName, std::ofstream::out | std::ofstream::trunc);
    #endif

    #ifdef VIS
        m_fps = static_cast<unsigned int>(rf.check("fps", yarp::os::Value(30)).asInt32());
        
        // Initiate the VIS matrices
        convolved_img = cv::Mat(m_height, m_width, CV_64F, cv::Scalar(0)); 

        // Create window for visualisation
        cv::namedWindow(getName(), cv::WINDOW_NORMAL);
        cv::resizeWindow(getName(), 800, 800);
        cv::waitKey(1);
        m = false;
    #endif
    
    // configure and start the baby thread
    asapThread.initialise(
            m_sae,
            m_img,
            convolved_img,
            m_kernel,
            m_alpha,
            &last_ts,
            getName(),
            m_height,
            m_width,
            &m
            #if LOG==1
                , &data
            #endif
            );

    yInfo() << getName() << " module configured";
    return Thread::start() && asapThread.start();
}

double LiteConv::getPeriod()
{
    return 1.0/m_fps; //period of synchrnous thread
}
                                             
bool LiteConv::interruptModule()
{
    return Thread::stop() && asapThread.stop();
}
                                              
void LiteConv::onStop()
{
    //close ports etc.
    m_inPort.close();   

    #if LOG==0
        for( auto d : data)
        {
            log << std::get<0>(d) << ", " << std::get<1>(d) << ", " << std::get<2>(d) << "\n";
        }
        log.close();
    #elif LOG==1
        for( auto d : data)
        {
            log << std::get<0>(d) << ", " << std::get<1>(d) << ", " << std::get<2>(d) << "\n";
        }
        log.close();
    #elif LOG==2
        log.close();
    #endif
            
    cv::destroyAllWindows();    
    return;
}

bool LiteConv::updateModule()
{
    return Thread::isRunning() && asapThread.isRunning();
}

void LiteConv::run()
{
    // using the vtsHelper for arith straight-forwardly was leanding to errors
    int maxTimestamp = static_cast<int>(ev::vtsHelper::max_stamp);
    
    Stamp yarpstamp;    
   
    int prev_tick = 0;
    
    while(true)
    {
        const vector<AE> * q = m_inPort.read(yarpstamp);
        if(!q || Thread::isStopping()) return;              

        #if LOG==1
            double tic = yarp::os::Time::now();
        #endif
        
        for(auto& qi:*q) // For each event
        {
            last_ts += ev::vtsHelper::deltaS(qi.stamp, prev_tick); 
            prev_tick = qi.stamp;
            
            // Calculate the decay
            double decay = exp(m_alpha*(m_sae.at<double>(qi.y, qi.x) - last_ts));
        
            if(qi.polarity)
                m_img.at<double>(qi.y, qi.x) = m_img.at<double>(qi.y, qi.x)*decay + 1;
            else
                m_img.at<double>(qi.y, qi.x) = m_img.at<double>(qi.y, qi.x)*decay - 1;

            m_sae.at<double>(qi.y, qi.x) = last_ts;

            #if VIS
                m = true;
            #endif

            #if LOG==2
                int idx = (int)(m_ksize-1)/2;
                int pi;
                if(qi.polarity) 
                    pi = 1;
                else
                    pi = -1;
                
                log << last_ts << ", " << convolved_img.at<double>(qi.y, qi.x) << ", " << convolved_img.at<double>(qi.y, qi.x)+pi*m_kernel.at<double>(idx, idx) << "\n";
            #endif
        } //for(auto& qi:*q)
        
        #if LOG==0
            data.push_back(std::tuple<double, double, double>(yarpstamp.getTime(), m_inPort.queryRate(), m_inPort.queryDelayT()));
        #elif LOG==1
            double avgtime = (yarp::os::Time::now()-tic)/q->size();
            data.push_back(std::tuple<int, double, double>(0, yarpstamp.getTime(), avgtime));
        #endif
    }
}
// Empty lines, the way gcc likes
