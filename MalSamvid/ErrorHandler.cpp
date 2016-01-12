#include "stdafx.h"

ErrorHandler::ErrorHandler()
{

}


ErrorHandler::~ErrorHandler()
{

}

void ErrorHandler::SetInformation(std::string s)
{
	errString = s;
}


std::string ErrorHandler::GetInformation()
{
	return errString ;
}

void ErrorHandler::LogError()
{

}

CmdException::CmdException(void)
{
}

CmdException::~CmdException(void)
{
}