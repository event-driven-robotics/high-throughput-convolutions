/******************************************************************************
                                                                              *
 * Copyright (C) 2019 Fondazione Istituto Italiano di Tecnologia (IIT)        *
 * All Rights Reserved.                                                       *
                                                                              *
                                                                              */

/**
 * @file src/refConv/refConv.cpp
 * @authors: Leandro de Souza Rosa <leandro.desouzarosa@iit.it>
 */

#include "refConv.h"

void Update::initialise(
        cv::Mat &m_sae,
        cv::Mat &m_img,
        double m_alpha,
        double *last_ts, 
        std::string m_name,
        unsigned int m_height,
        unsigned int m_width,
        unsigned int m_padSize,
        bool *m
        #if LOG==1
            , std::vector<std::tuple<int, double, double>> *data
        #endif
)
{
    sae = m_sae;
    img = m_img;
    alpha = m_alpha;
    img_ts = last_ts; 
    name = m_name;
    mlock = m;
    width = m_width;
    height = m_height;
    padSize = m_padSize;
    #if LOG==1
        d = data;
    #endif
   
    coefs = cv::Mat(height, width, CV_64F, cv::Scalar(0));
    decays = cv::Mat(height, width, CV_64F, cv::Scalar(0)); 
    updated_img = cv::Mat(height, width, CV_64F, cv::Scalar(0));
    norm_img = cv::Mat(height, width, CV_64F, cv::Scalar(0)); 
}

void Update::run()
{
    while(!isStopping())
    {
        if(*mlock){
            #ifdef VIS
                #if LOG==1
                    double tic = yarp::os::Time::now();
                #endif
                
                // calculate the exponent of the decay for the whole img
                coefs = alpha*(sae(cv::Rect(padSize, padSize, width, height)) - *img_ts);
                             
                // compute the decays = e^(-alpha*dt) - saves on "decays" mat
                cv::exp(coefs, decays);
                
                // create updated image by decaying all pixels to the last ts
                updated_img = img(cv::Rect(padSize, padSize, width, height)).mul(decays);
               
                // TODO: tic is not working properly for exporting to python 
                #if LOG==1
                    d->push_back(std::tuple<int, double, double>(1, tic, (yarp::os::Time::now()-tic)));
                #endif
                
                normalize(updated_img, norm_img, 0, 1, NORM_MINMAX);
                cv::imshow(name, (1-norm_img));// invert colours
                cv::waitKey(1);
                
                *mlock = false;
            #endif                                     
        } // if mlock
    }// while !isStopping()
}// run()

bool RefConv::configure(yarp::os::ResourceFinder& rf)
{
    
    // open yarp ports (in/out) associated to the module
    setName((rf.check("name", yarp::os::Value("/refConv")).asString()).c_str());

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
    m_padSize = static_cast<int>((m_ksize-1)/2);

    // Convolved image
    // pads facilitate the border management
    m_img = cv::Mat(m_height + 2*m_padSize, m_width + 2*m_padSize, CV_64F, cv::Scalar(0));
    // SAE
    m_sae = cv::Mat(m_height + 2*m_padSize, m_width + 2*m_padSize, CV_64F, cv::Scalar(0));

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
           m_alpha,
           &last_ts,
           getName(),
           m_height,
           m_width,
           m_padSize,
           &m
           #if LOG==1
               , &data
           #endif
           );

    yInfo() << getName() << "module configured";
    return Thread::start() && asapThread.start();
}

double RefConv::getPeriod()
{
    return 1.0/m_fps; //period of synchrnous thread
}
                                             
bool RefConv::interruptModule()
{
    return Thread::stop() && asapThread.stop();
}

void RefConv::onStop()                                               
{

    //close ports etc.
    m_inPort.close();   
    //m_inPort.releaseDataLock(); # Cant remember why we needed that
    #ifdef VIS
        m = false;
    #endif
    
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


bool RefConv::updateModule()
{
    return Thread::isRunning() && asapThread.isRunning();
}

void RefConv::run()
{
    // using the vtsHelper for arith straight-forwardly was leanding to errors
    int maxTimestamp = static_cast<int>(ev::vtsHelper::max_stamp);
    
    Stamp yarpstamp;    
    
    // Allocating openCV matrices
    cv::Mat saePatch = cv::Mat(m_ksize, m_ksize, CV_64F);
    cv::Mat fsaePatch = cv::Mat(m_ksize, m_ksize, CV_64F);
    cv::Mat coefs  = cv::Mat(m_ksize, m_ksize, CV_64F);
    cv::Mat decay = cv::Mat(m_ksize, m_ksize, CV_64F);
    cv::Mat img_window = cv::Mat(m_ksize, m_ksize, CV_64F);
    
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

            // Pad reminder: (xi,yi) in the SAE is the kernel starting point, not its center
            // Get a reference to the SAE patch referring to the kernel
            saePatch = m_sae(cv::Rect(qi.x, qi.y, m_ksize, m_ksize));
            
            // decay coefficient = -alpha*delta-time
            coefs = m_alpha*(saePatch - last_ts);

            // Calculate the decay
            cv::exp(coefs, decay);

            // Update SAE 
            saePatch = last_ts;

            // Pad reminder: (xi,yi) in the image is the kernel starting point, not its center
            // get a reference to the image region referring to the kernel
            img_window = m_img(cv::Rect(qi.x, qi.y, m_ksize, m_ksize));
            // Decay the img and sum the current kernel
            if(qi.polarity)
                img_window = img_window.mul(decay) + m_kernel;
            else
                img_window = img_window.mul(decay) - m_kernel;

            #if VIS
                m = true;
            #endif

            #if LOG==2
                double energy = cv::norm(img_window, NORM_L1);
                log << last_ts << ", " << m_img.at<double>(qi.y+m_padSize, qi.x+m_padSize) << ", " << energy << "\n";
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
