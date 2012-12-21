/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <math.h>

#include "odblib/FunctionFactory.h"
#include "odblib/FunctionIntegerExpression.h"
#include "odblib/Codec.h"

#define RMDI   -2147483647
#define NMDI    2147483647
#define ftrunc(x) ((x) -fmod((x), 1))
#define F90nint(x) ( ((x) > 0) ? (int)((x) + 0.5) : (int)((x) - 0.5) )

namespace odb {
namespace sql {
namespace expression {
namespace function {

FunctionIntegerExpression::FunctionIntegerExpression(const string& name,const expression::Expressions& args)
: FunctionExpression(name,args)
{}

FunctionIntegerExpression::~FunctionIntegerExpression() {}

const odb::sql::type::SQLType* FunctionIntegerExpression::type() const
{
	return &odb::sql::type::SQLType::lookup("integer");
}

void FunctionIntegerExpression::output(ostream& s) const
{
	bool missing;
    double v = eval(missing);

	//Log::info() << "FunctionIntegerExpression::output: v=" << v << ", missing=" << missing << endl;

    s << static_cast<long long int>(v);
}

//===============================

#include <math.h>

template<double (*T)(double)> 
class MathFunctionIntegerExpression_1 : public FunctionIntegerExpression {
	double eval(bool& m) const {
		double v = args_[0]->eval(m);
		return m ? this->missingValue_ : T(v);
	}
	SQLExpression* clone() const { return new MathFunctionIntegerExpression_1<T>(this->name_,this->args_); }
public:
	MathFunctionIntegerExpression_1(const string& name,const expression::Expressions& args)
	: FunctionIntegerExpression(name, args) { this->missingValue_ = MISSING_VALUE_INT; }
};

#define DEFINE_MATH_INT_FUNC_1F(FuncName, Name, Help) \
static FunctionMaker<MathFunctionIntegerExpression_1<FuncName> > make_1_##FuncName(#Name,1,Help)


//--------------------------------------------------------------
inline double year(double x) { return ((fabs(x) != fabs((double) RMDI)) ? (double)((int)((x)/10000)) : (double)NMDI);}
inline double month(double x) { return ((fabs(x) != fabs((double) RMDI)) ? (double)(((int)((x)/100))%100) : (double)NMDI);}
inline double day(double x)   { return ((fabs(x) != fabs((double) RMDI)) ? (double)(((int)(x))%100) : (double)NMDI);}
inline double hour(double x)   {return ((fabs(x) != fabs((double) RMDI)) ? (double)((int)((x)/10000)) : (double)NMDI);}
inline double minute(double x) {return ((fabs(x) != fabs((double) RMDI)) ? (double)(((int)((x)/100))%100) : (double)NMDI);}
inline double minutes(double x) {return minute(x);}
inline double second(double x) {return ((fabs(x) != fabs((double)RMDI)) ? (double)(((int)(x))%100) : (double)NMDI);}
inline double seconds(double x) {return second(x);}


inline double Func_ftrunc(double x) { return ((fabs(x) != fabs((double) RMDI)) ? (double)(ftrunc(x)) : (double) NMDI); }
inline double Func_dnint(double x) { return ((fabs(x) != fabs((double) RMDI)) ? (double)(F90nint(x)) : (double)NMDI); }
inline double Func_dint(double x) { return ((fabs(x) != fabs((double) RMDI)) ? (double)(ftrunc(x)) : (double) NMDI);}
inline double Func_ceil(double x) { return ((fabs(x) != fabs((double) RMDI)) ? (double)(ceil(x)) : (double) NMDI);}
inline double Func_floor(double x) { return ((fabs(x) != fabs((double) RMDI)) ? (double)(floor(x)) : (double) NMDI);}

void FunctionIntegerExpression::registerIntegerFunctions()
{

	DEFINE_MATH_INT_FUNC_1F(year,year,"");
	DEFINE_MATH_INT_FUNC_1F(month,month,"");
	DEFINE_MATH_INT_FUNC_1F(day,day,"");
	DEFINE_MATH_INT_FUNC_1F(hour,hour,"");
	DEFINE_MATH_INT_FUNC_1F(minute,minute,"");
	DEFINE_MATH_INT_FUNC_1F(minutes,minutes,"");
	DEFINE_MATH_INT_FUNC_1F(second,second,"");
	DEFINE_MATH_INT_FUNC_1F(seconds, seconds,"");
	DEFINE_MATH_INT_FUNC_1F(Func_ceil,ceil,"");
	DEFINE_MATH_INT_FUNC_1F(Func_floor,floor,"");
	DEFINE_MATH_INT_FUNC_1F(Func_ftrunc,trunc,"");
	DEFINE_MATH_INT_FUNC_1F(Func_dint,int,"");
	DEFINE_MATH_INT_FUNC_1F(Func_dnint,nint,"");
}

} // namespace function
} // namespace expression
} // namespace sql
} // namespace odb

