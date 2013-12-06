/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "odblib/SQLStatement.h"

namespace odb {
namespace sql {

SQLStatement::SQLStatement() {}

SQLStatement::~SQLStatement() {}

void SQLStatement::print(std::ostream& s) const {}

} // namespace sql
} // namespace odb
