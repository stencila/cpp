#pragma once

#include <fstream>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <stencila/exception.hpp>
#include <stencila/mirror.hpp>

namespace Stencila {
namespace Mirrors {

class RowHeader : public Mirror<RowHeader>, public std::string {
public:
	RowHeader(const std::string& separator="\t"):
		separator_(separator){}

	template<typename Data>
	RowHeader& data(Data& data, const std::string& name=""){
		if(length()>0) append(separator_);
		append(name);
		return *this;
	}

private:
	std::string separator_;
}; // class RowHeader


class RowGenerator : public Mirror<RowGenerator>, public std::string {
public:
	RowGenerator(const std::string& separator="\t"):
		separator_(separator){}

	template<typename Data>
	RowGenerator& data(Data& data, const std::string& name=""){
		if(length()>0) append(separator_);
		append(boost::lexical_cast<std::string>(data));
		return *this;
	}

private:
	std::string separator_;
}; // class RowGenerator

class RowWriter : public Mirror<RowWriter>, public std::ofstream {
public:

	RowWriter(const std::string& path, const std::vector<std::string>& prefixes = {}, const std::vector<std::string>& names = {},const std::string& separator="\t"):
		path_(path),
		prefixes_(prefixes),
		names_(names),
		separator_(separator),
		file_(path){
		// All `data` attributes?
		all_ = names.size()==0;
	}

	template<typename Type>
	RowWriter& start(Type& type){
		// Write the header row
		for(auto prefix : prefixes_) file_<<prefix<<separator_;
		if(all_) file_<<RowHeader(separator_).mirror(type);
		else {
			for(auto name : names_) file_<<name<<separator_;
		}
		file_<<"\n";
		return *this;
	}

	template<typename Data>
	RowWriter& data(Data& data, const std::string& name=""){
		bool write = false;
		if(all_) write = true;
		else write = std::find(names_.begin(), names_.end(), name) != names_.end();
		if(write){
			if(started_) file_<<separator_;
			else started_ = true;
			file_<<data;
		}
		return *this;
	}

	template<class Reflector, typename... Args>
	RowWriter& write(Reflector& reflector, Args... args){
		started_ = false;
		write_prefixes_(args...);
		reflector.reflect(*this);
		file_<<"\n";
		return *this;
	}

private:
	bool all_;
	bool started_;
	std::string path_;
	std::vector<std::string> prefixes_;
	std::vector<std::string> names_;
	std::string separator_;
	std::ofstream file_;

	template<
		typename Arg,
		typename... Args
	>
	void write_prefixes_(Arg arg, Args... args){
		file_<<arg<<separator_;
		started_ = true;
		write_prefixes_(args...);
	}

	void write_prefixes_(void){
	}
}; // class RowWriter

class RowParser : public Mirror<RowParser> {
public:
	RowParser(const std::string& row,const std::string& separator="\t"){
		boost::split(items_, row, boost::is_any_of(separator));
	}

	template<typename Data>
	RowParser& data(Data& data, const std::string& name=""){
		if(index_>=items_.size()) STENCILA_THROW(Exception,
			"Not enough elements in row; got <"+boost::lexical_cast<std::string>(items_.size())+">, need at least <"+boost::lexical_cast<std::string>(index_)+">"
		);
		data = boost::lexical_cast<Data>(items_[index_]);
		index_++;
		return *this;
	}

private:
	std::vector<std::string> items_;
	unsigned int index_ = 0;
}; // class RowParser

class ColumnMatcher : public Mirror<ColumnMatcher> {
public:
	ColumnMatcher(void){}

	ColumnMatcher(const std::string& names,const std::string& values, const std::string& separator="\t"){
		this->names(names);
		this->values(values);
		if(names_.size()!=values_.size()){
			auto message = 
				"Different numbers of names and values; got <" + boost::lexical_cast<std::string>(names_.size()) + "> " +
				"names and <" + boost::lexical_cast<std::string>(values_.size()) + "> " +
				"values using separator <" + separator + "> ";
			STENCILA_THROW(Exception,message);
		}
	}

	ColumnMatcher& names(const std::string& names, const std::string& separator="\t"){
		boost::split(names_, names, boost::is_any_of(separator));
		return *this;
	}

	ColumnMatcher& values(const std::string& values, const std::string& separator="\t"){
		boost::split(values_, values, boost::is_any_of(separator));
		return *this;
	}

	template<typename Data>
	ColumnMatcher& data(Data& data, const std::string& name=""){
		auto iter =  std::find(names_.begin(), names_.end(), name);
		if(iter!=names_.end()){
			unsigned int index = iter-names_.begin();
			data = boost::lexical_cast<Data>(values_[index]);
		}
		return *this;
	}

private:
	std::vector<std::string> names_;
	std::vector<std::string> values_;
}; // class ColumnMatcher

} // namepace Mirrors
} // namespace Stencila
