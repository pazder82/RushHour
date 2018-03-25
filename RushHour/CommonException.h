#pragma once
#include <stdexcept>

class CommonException : public std::exception {
public:
	CommonException() = default;
	CommonException(LPWSTR msg) : _msg(msg), exception() {}
	~CommonException() = default;

	virtual LPWSTR What() { return _msg; }

private:
	LPWSTR _msg;
};

