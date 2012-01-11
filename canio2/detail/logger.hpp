/****************************************************************************
 *                                                                          *
 *                            CANIO2 Lib Project.                           *
 *                                                                          *
 ****************************************************************************/
/**
 @file 
 ****************************************************************************
 @brief		Simple logger
 ****************************************************************************
 @details	Simple logger with string stream capabilities.
			fprintf is used, so calls are thread safe.
			TODO: customizable log to log into GUI control or file, etc...
 ****************************************************************************
 @author    gruzovator
 ****************************************************************************/

#ifndef LOGGER_HPP_
#define LOGGER_HPP_
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

#ifdef DEBUG
#define LOG_DBG_(s)\
	{std::ostringstream os__;\
	os__ << s ;\
    fprintf(stdout, "%s\n", os__.str().c_str() );}
#else
    #define LOG_DBG_(s)
#endif

#define LOG_ERR_(s)\
	{std::ostringstream os__;\
	os__ << s ;\
	fprintf(stdout, "%s\n", os__.str().c_str() );}

#define LOG_ERR_THROW_(s)\
    {std::ostringstream os__;\
    os__ << s ;\
    fprintf(stdout, "%s\n", os__.str().c_str() );\
    throw std::runtime_error(os__.str().c_str());}

#endif /* LOGGER_HPP_ */

