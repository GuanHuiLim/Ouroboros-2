#pragma once
#include <string>
class InputManagerUI
{
public:
	InputManagerUI();
	~InputManagerUI();
	void Show();
private:
	bool DrawInputTypeUI(int& curr);
	bool DrawKeyInputUI(const std::string& name,int& curr,bool mouse);
};
