/****************************************************************************
 *                                                                          *
 *                            CANIO2 Lib Project.                           *
 *                                                                          *
 ****************************************************************************/
/**
 @file
 ****************************************************************************
 @brief     Python Library
 ****************************************************************************
 @details
 ****************************************************************************
 @author    gruzovator
 ****************************************************************************/
#ifndef PY_HPP
#define PY_HPP

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/enum.hpp>
#include <boost/python/make_function.hpp>
#include <boost/mpl/vector.hpp>
#include "canio2.hpp"

using namespace boost::python;
using namespace CANIO2;

void export_pmd();
void export_canopen();

#endif // PY_HPP
