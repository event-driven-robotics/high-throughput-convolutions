/*****************************************************************
 * Copyright (C) 2019 Fondazione Istituto Italiano di Tecnologia (IIT)
*/
/**
 * @file src/liteConv/liteConv.h
 * @authors: Leandro de Souza Rosa <leandro.desouzarosa@iit.it>
 */

#ifndef __LITE_CONV_H
#define __LITE_CONV_H

#include <yarp/os/all.h>
#include <event-driven/all.h>

#include <vector>
#include <iterator>

#include <fstream>
#include <iomanip>      // std::setprecision

#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d.hpp> // ORB descriptor and feature

#define _USE_MATH_DEFINES 
#include <cmath>

using namespace ev;
using namespace cv;
using namespace yarp::os;
using namespace std;

#if LOG==0 || LOG==1 || LOG==2
    #include <experimental/filesystem>
    namespace fs = std::experimental::filesystem;
#endif

/**
 * @class LiteConv
 * @ Implements the ASAP update and convolution for our Lite Convolution method
 *
 * @file src/liteConv/liteConv.h
 *
 * @author Leandro de Souza Rosa (15/Apr/2021)
 */
class UpdateAndConvolve : public Thread {

public:
    cv::Mat sae, img, convolved, kernel;
    std::string name;
    double alpha;
    double *img_ts;
    cv::Mat coefs, decays, updated_img, norm_img; 
    bool *mlock;
    std::vector<std::tuple<int, double, double>> *d;

    void initialise(
            cv::Mat &m_sae,
            cv::Mat &m_img,
            cv::Mat &convolved_img,
            cv::Mat &m_kernel,
            double m_alpha,
            double *last_ts, std::string m_name,
            unsigned int m_height,
            unsigned int m_width,
            bool *m
            #if LOG==1
                , std::vector<std::tuple<int, double, double>> *data
            #endif
    );
    
    void run();
};

/**
 * @class LiteConv
 * @brief Implements our Lite Convolution method
 *
 * @file src/liteConv/liteConv.h
 *
 * @author Leandro de Souza Rosa (16/Mar/2021)
 */
class LiteConv : public RFModule, public Thread {

public:
    double getPeriod();
                                                  
    bool interruptModule();                
    
    /*!
     * Stop the thread
     */
    void onStop();

    /*!
     * TODO: describe the method run()
     */
    void run(); //asynchronous thread

    /*!
     * Open and configure all the resources.
     *
     * \param rf contains command-line options.
     *
     * \return bool true/false iff success/fail.
     */
    virtual bool configure(yarp::os::ResourceFinder& rf);

    /*!
     * Background service thread (synchronous).
     *
     * \return bool true/false iff success/fail.
     */
    bool updateModule();

private:
    vReadPort< vector<AE> > m_inPort; //!< port to receive the events
    
    unsigned int m_width; //!< image width
    unsigned int m_height; //!< image height

    cv::Mat m_img; //!< Matrix that the convolved image 
    cv::Mat m_sae; //!< Saves the timestamp of the last event in each image position
    double last_ts{0.0}; //!< timestamp of the last event
    
    double m_alpha; //!< Cut frequency for high-pass filter
    unsigned int m_ksize; //!< convolution kernel size
    double m_sigma; //!< convolution kernel sigma 
    cv::Mat m_kernel; //!< convolution kernel

    #if LOG==0 || LOG==1 || LOG==2
        std::string logFileName; //<! path to the scores log file
        fs::path logFilePath;
        std::ofstream log;
    #endif
    
    #if LOG ==0 
        std::vector<std::tuple<double, double, double>> data;
    #elif LOG==1
        std::vector<std::tuple<int, double, double>> data;
    #elif LOG==2
        //
    #endif
    
    // baby thread    
    UpdateAndConvolve asapThread;

    #if VIS
        cv::Mat coefs;
        cv::Mat decays;
        cv::Mat updated_img;
        cv::Mat norm_img;
        cv::Mat convolved_img;
        bool m{false};
        unsigned int m_fps;
    #endif
};

#endif
//empty line to make gcc happy
