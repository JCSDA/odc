/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// \file .h
///
/// @author Piotr Kuchta, ECMWF, Feb 2009

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

#include <stdlib.h>
#include <memory> 

using namespace std;

#include "eclib/PathName.h"
#include "eclib/DataHandle.h"
#include "eclib/Log.h"
#include "eclib/DataHandle.h"
#include "eclib/PartFileHandle.h"
#include "eclib/ThreadPool.h"

#include "odblib/DataStream.h"
#include "odblib/HashTable.h"
#include "odblib/Codec.h"
#include "odblib/HashTable.h"
#include "odblib/Column.h"
#include "odblib/MetaData.h"
#include "odblib/RowsIterator.h"
#include "odblib/HashTable.h"
#include "odblib/SQLBitfield.h"
#include "odblib/SQLAST.h"
#include "odblib/SchemaAnalyzer.h"
#include "odblib/SQLIteratorSession.h"
#include "odblib/Header.h"
#include "odblib/Reader.h"
#include "odblib/SelectIterator.h"
#include "odblib/ReaderIterator.h"
#include "odblib/Comparator.h"
#include "odblib/Decoder.h"
#include "odblib/SQLSelectFactory.h"
#include "odblib/FastODA2Request.h"
#include "odblib/MemoryBlock.h"
#include "odblib/InMemoryDataHandle.h"
#include "odblib/oda.h"
#include "odblib/odbcapi.h"
#include "odblib/ImportTool.h"
#include "odblib/Tool.h"
#include "odblib/TestCase.h"
#include "odblib/DateTime.h"
#include "odblib/SplitTool.h"
#include "odblib/CountTool.h"
#include "odblib/MapReduce.h"


#include "eclib/ThreadControler.h"
#include "eclib/Thread.h"
#include "eclib/Mutex.h"
#include "eclib/MutexCond.h"
#include "eclib/AutoLock.h"

namespace odb {
namespace tool {

template <typename CALLBACK>
class PartFileProcessor : public ThreadPoolTask {
public:
	PartFileProcessor(const PathName& fileName, const Offset& offset, const Length& length,
						void* userData, const string& sql, CALLBACK callBack) 
	: dh_(new PartFileHandle(fileName, offset, length)),
	  userData_(userData),
	  sql_(sql),
	  callBack_(callBack), 
	  fileName_(fileName),
	  offset_(offset),
	  length_(length)
	{}

	~PartFileProcessor() {}

	void execute()
	{
		//Log::info() << "Opening [" << fileName_ << "] <" << offset_ << "," << length_ << ">" << endl;
		dh_->openForRead();
		SingleThreadMapReduce::process(userData_, *dh_, sql_, callBack_);
		//Log::info() << "Closing [" << fileName_ << "] <" << offset_ << "," << length_ << ">" << endl;
		dh_->close();
	}
private:
	auto_ptr<DataHandle> dh_;
	void* userData_;
	const string sql_;
	CALLBACK callBack_;
	
	PathName fileName_;
	Offset offset_;
	Length length_;
};


void * SingleThreadMapReduce::process(void* userData, DataHandle& dh, const string& sql, CallBackProcessOneRow f)
{
	if (userData == 0) userData = f.create(); 
	odb::Select o(sql, dh);
	odb::Select::iterator it(o.begin()), end(o.end());
	for (; it != end; ++it)
		f.mapper(userData, it->data(), it->columns().size());

	return userData;
}

void * SingleThreadMapReduce::process(void* userData, const PathName& fileName, const string& sql, CallBackProcessOneRow f)
{
	FileHandle dh(fileName);
	Length estimate = dh.openForRead();
	return process(userData, dh, sql, f);
}

const size_t SingleThreadMapReduce::N = 64;
const size_t MultipleThreadMapReduce::threadPoolSize_ = 5;

void * SingleThreadMapReduce::process(void* userData, DataHandle& dh, const string& sql, CallBackProcessArray f)
{
	odb::Select o(sql, dh);
	odb::Select::iterator it(o.begin()), end(o.end());
	size_t nCols = it->columns().size();
	double a[N][nCols];

	void * result = f.create();
	for (; it != end; )
	{
		const double *data = 0;
		size_t i = 0, j = 0;
		for (; i < N; ++i)
		{
			data = it->data();
			j = 0;
			for (; j < nCols; ++j)
				a[i][j] = data[j];
			if (it != end)
				++it;
			else break;
		}
		Array array;
		array.data = data;
		array.nCols = j;
		array.nRows = i;
		// TODO: reduce
		f.mapper(result, array);
	}
	return result;
}

void * SingleThreadMapReduce::process(void* userData, const PathName& fileName, const string& sql, CallBackProcessArray f)
{
	FileHandle dh(fileName);
	Length estimate = dh.openForRead();
	return SingleThreadMapReduce::process(userData, dh, sql, f);
}

void * MultipleThreadMapReduce::process(void* userData, const PathName& fileName, const string& sql, CallBackProcessArray f)
{ 
	ThreadPool pool(string("[") + fileName + " processors]", threadPoolSize_);
	vector<void *> results;
	vector<pair<Offset,Length> > chunks(SplitTool::getChunks(fileName));
    for(size_t i=0; i < chunks.size(); ++i)
	{   
		pair<Offset,Length>& chunk(chunks[i]);
		//Log::info() << "MultipleThreadMapReduce::process: process chunk " << chunk.first << "," << chunk.second << endl;

		void *result = f.create();
		pool.push(new PartFileProcessor<CallBackProcessArray>(fileName, chunk.first, chunk.second, result, sql, f));
		results.push_back(result);
    }

	pool.waitForThreads();

	//if (userData == 0) userData = f.create(); 
	
	void *r = f.create();
	for (size_t i = 0; i < results.size(); ++i)
	{
		void *oldResult = r;
		r = f.reducer(results[i], oldResult);
		f.destroy(oldResult);
	}

	return r;
} 

void * MultipleThreadMapReduce::process(void* userData, DataHandle& dh, const string& sql, CallBackProcessOneRow f)
{ NOTIMP; return 0; }

void * MultipleThreadMapReduce::process(void* userData, const PathName& fileName, const string& sql, CallBackProcessOneRow f)
{
	ThreadPool pool(string("[") + fileName + " processors]", threadPoolSize_);

	vector<void *> results;
	vector<pair<Offset,Length> > chunks(SplitTool::getChunks(fileName));
    for(size_t i=0; i < chunks.size(); ++i)
    {   
		pair<Offset,Length>& chunk(chunks[i]);
		//Log::info() << "MultipleThreadMapReduce::process: process chunk " << chunk.first << "," << chunk.second << endl;

		void *result = f.create();
		pool.push(new PartFileProcessor<CallBackProcessOneRow>(fileName, chunk.first, chunk.second, result, sql, f));
		results.push_back(result);
    } 
	pool.waitForThreads();

	//if (userData == 0) userData = f.create(); 
	
	void *r = f.create();
	for (size_t i = 0; i < results.size(); ++i)
	{
		void *oldResult = r;
		r = f.reducer(results[i], oldResult);
		f.destroy(oldResult);
	}

	return r;
}


template<class PAYLOAD>
class Producer {
public:
    virtual bool done() = 0;
    virtual void produce(PAYLOAD&) = 0;
};


template<class PAYLOAD>
class Consumer {
public:
    virtual void consume(PAYLOAD&) = 0;
};

template<class PAYLOAD>
struct OnePayload {
    MutexCond cond_;
    bool      ready_;
    bool       done_;
    PAYLOAD   payload_;
    OnePayload(): ready_(false), done_(false), payload_() {}
};

template<class PAYLOAD>
class ProducerConsumer;

template<class PAYLOAD>
class ProducerConsumerTask : public Thread {
    ProducerConsumer<PAYLOAD>& owner_;
    Consumer<PAYLOAD>&         consumer_;
    OnePayload<PAYLOAD>*       payloads_;
public:
    ProducerConsumerTask(Consumer<PAYLOAD>&,ProducerConsumer<PAYLOAD>&,OnePayload<PAYLOAD>*);
    virtual void run();
};


template<class PAYLOAD>
class ProducerConsumer {
public:
    ProducerConsumer(long count = 2);
    ~ProducerConsumer();

    void execute(Producer<PAYLOAD>&, Consumer<PAYLOAD>&);

    bool error();
    void error(const string&);

private:
    ProducerConsumer(const ProducerConsumer&);
    ProducerConsumer& operator=(const ProducerConsumer&);

    Mutex mutex_;
    long count_;

    bool error_;
    string why_;

    friend class ProducerConsumerTask<PAYLOAD>;
};


template<class PAYLOAD>
ProducerConsumer<PAYLOAD>::ProducerConsumer(long count)
: count_(count), error_(false)
{}

template<class PAYLOAD>
ProducerConsumer<PAYLOAD>::~ProducerConsumer()
{}

template<class PAYLOAD>
inline void ProducerConsumer<PAYLOAD>::error(const string& why)
{
    AutoLock<Mutex> lock(mutex_);
    error_ = true;
    why_   = why;
}

template<class PAYLOAD>
inline bool ProducerConsumer<PAYLOAD>::error()
{
    AutoLock<Mutex> lock(mutex_);
    return error_;
}


template<class PAYLOAD>
void ProducerConsumer<PAYLOAD>::execute(Producer<PAYLOAD>& producer,Consumer<PAYLOAD>& consumer)
{
    static const char *here = __FUNCTION__;

    OnePayload<PAYLOAD>* payloads = new OnePayload<PAYLOAD>[count_];

    error_  = false;

    ThreadControler thread(new ProducerConsumerTask<PAYLOAD>(consumer, *this, payloads), false);

    thread.start();

    int i = 0;

    while(!error())
    {
        AutoLock<MutexCond> lock(payloads[i].cond_);

        while(payloads[i].ready_)
            payloads[i].cond_.wait();

        if(error())
            break;

        if(producer.done())
        {
            payloads[i].done_ = true;
            Log::info() << "Producer done" << endl;
            payloads[i].ready_ = true;
            payloads[i].cond_.signal();
            break;
        }

        try {
            producer.produce(payloads[i].payload_);
        }
        catch(exception& e)
        {
            Log::error() << "** " << e.what() << " Caught in " << here <<  endl;
            Log::error() << "** Exception is handled" << endl;
            error(e.what());
        }

        payloads[i].ready_ = true;
        payloads[i].cond_.signal();

        i++;
        i %= count_;
    }


    thread.wait();
    delete[] payloads;

    if(error_) {
        throw SeriousBug(why_);
    }


}

template<class PAYLOAD>
ProducerConsumerTask<PAYLOAD>::ProducerConsumerTask(Consumer<PAYLOAD>& consumer,
	ProducerConsumer<PAYLOAD>& owner, OnePayload<PAYLOAD>* payloads)
: Thread(false),
  consumer_(consumer),
  owner_(owner),
  payloads_(payloads)
{}

template<class PAYLOAD>
void ProducerConsumerTask<PAYLOAD>::run()
{
    static const char *here = __FUNCTION__;
    int i = 0;

    while(!owner_.error())
    {
        AutoLock<MutexCond> lock(payloads_[i].cond_);

        while(!payloads_[i].ready_)
            payloads_[i].cond_.wait();

        if(owner_.error())
            break;

        if(payloads_[i].done_) {
            payloads_[i].ready_ = false;
            payloads_[i].cond_.signal();
            break;
        }

        bool error = false;

        try {
            consumer_.consume(payloads_[i].payload_);
        }
        catch(exception& e)
        {
            Log::error() << "** " << e.what() << " Caught in " <<
                            here <<  endl;
            Log::error() << "** Exception is handled" << endl;
            owner_.error(e.what());
            error = true;
        }

         payloads_[i].ready_ = false;

        if(error)
        {
            ASSERT(owner_.error());
            payloads_[i].cond_.signal();
            break;
        }


        payloads_[i].cond_.signal();

        i++;
        i %= owner_.count_;
    }

    Log::info() << "End producer" << endl;
}

struct Data {
	double data[1024][1024];
	size_t nRows;
	size_t nCols;
};


struct C : public Consumer<Data> {

    virtual void consume(Data& s) {
		for (size_t i = 0; i < s.nCols; ++i)
			for (size_t j = 0; j < s.nRows; ++j)
				s.data[i][j] = sin(s.data[i][j]);	
    }

};

struct P : public Producer<Data> {
	P(odb::Select& o) : it_(o.begin()), end_(o.end()) {}

    virtual bool done() { return ! (it_ != end_); }

    virtual void produce(Data& s)
	{
		size_t nCols = it_->columns().size();
		size_t count = 1024;

		s.nCols = nCols;
		s.nRows = 0;

		for (; it_ != end_ && count > 0; --count, ++s.nRows, ++it_)
		{
			const double *data = it_->data();
			for (size_t i =0; i < nCols; ++i)
				s.data[i][s.nRows] = data[i];
		}
	}

	odb::Select::iterator it_;
	odb::Select::iterator end_;
};


void producer_consumer()
{
	Timer timer("ALL");
	using namespace odb::tool;
	string fileName = "/scratch/ma/mak/odb-16/all.odb";
	odb::Select o(string("select lat,lon,obsvalue,sin(obsvalue) from \"") + fileName + "\"");
	P p(o); C c;
	ProducerConsumer<Data> pc(1);
	pc.execute(p, c);
}

} // namespace tool 
} // namespace odb 
