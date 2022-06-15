/******************************************************************************
                                                                              *
 * Copyright (C) 2019 Fondazione Istituto Italiano di Tecnologia (IIT)        *
 * All Rights Reserved.                                                       *
                                                                              *
                                                                              */

/**
 * @file src/refConv/refConv.h
 * @authors: Leandro de Souza Rosa <leandro.desouzarosa@iit.it>
 */

#ifndef __REF_CONV_H
#define __REF_CONV_H

#include <yarp/os/all.h>
#include <event-driven/all.h>

#include <vector>
#include <iterator>
#include <algorithm> // std min and max

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
class Update : public Thread {

public:
    cv::Mat sae, img;
    std::string name;
    double alpha;
    double *img_ts;
    cv::Mat coefs, decays, updated_img, norm_img; 
    bool *mlock;
    std::vector<std::tuple<int, double, double>> *d;
    unsigned int width, height, padSize;

    void initialise(
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
    );
    
    void run();
};

/**
 * @class RefConv
 * @brief Implements the event-by-event convolution presented in Scheerlinck (2019)
 *
 * @file src/refConv/refConv.h
 *
 * @:author Leandro de Souza Rosa (10/Mar/2021)
 */
class RefConv : public RFModule, public Thread {

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
    unsigned int m_padSize; //!< kernel ofsset from centre

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
    Update asapThread;

    #if VIS
        cv::Mat coefs; //no pad img size
        cv::Mat decays; // idem
        cv::Mat updated_img; // idem
        cv::Mat norm_img; //idem
        bool m{false};
        unsigned int m_fps;
    #endif
};

#endif
//empty line to make gcc happy
