#pragma once
class ErrorHandler
{
private:
	std::string errString;
public:

	void SetInformation(std::string s);
	std::string GetInformation();
	ErrorHandler(void);
	~ErrorHandler(void);

	void LogError();

};

class CmdException:
	public ErrorHandler
{
public:
	CmdException(void);
	~CmdException(void);
};