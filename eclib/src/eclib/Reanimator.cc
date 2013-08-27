/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "eclib/Streamable.h"


//-----------------------------------------------------------------------------

namespace eclib {

//-----------------------------------------------------------------------------

template<class T> 
Reanimator<T>::Reanimator():
	 ReanimatorBase(T::classSpec())
{
}

template<class T> 
Streamable* Reanimator<T>::ressucitate(Stream& s) const
{
	return new T(s);
}


template<class T>
T*  Reanimator<T>::reanimate(Stream& s) 
{ 
	return (T*)ReanimatorBase::reanimate(s,&T::classSpec()); 
}

//-----------------------------------------------------------------------------

} // namespace eclib

