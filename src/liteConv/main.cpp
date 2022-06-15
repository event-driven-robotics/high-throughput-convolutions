/******************************************************************************
                                                                              *
 * Copyright (C) 2019 Fondazione Istituto Italiano di Tecnologia (IIT)        *
 * All Rights Reserved.                                                       *
                                                                              *
                                                                              */

/**
 * @file src/liteConv/main.cpp
 * @authors: Leandro de Souza Rosa <leandro.desouzarosa@iit.it>
 */

#include "liteConv.h"

int main(int argc, char * argv[])
{
    /* initialize yarp network */
    yarp::os::Network yarp;
    if(!yarp.checkNetwork(2)) {
        std::cout << "Could not connect to YARP" << std::endl;
        return -1;
    }

    /* prepare and configure the resource finder */
    yarp::os::ResourceFinder rf;
    rf.setVerbose(false);
    rf.setDefaultContext("paper_convolution");
    //rf.setDefaultConfigFile("liteConv.ini");
    rf.configure(argc, argv);

    /* create the module */
    LiteConv liteConv;

    /* run the module: runModule() calls configure first and, if successful, then it runs */
    return liteConv.runModule(rf);
}
