#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <iostream>
#include <fstream>
using namespace std;
#include "curl/curl.h"
#pragma comment(lib,"libcurl_imp.lib")
//---------------------------------------------------------------------------
std::string escape(CURL* curl_handle, const std::string& text)
{
	std::string result;
	char* esc_text = curl_easy_escape(curl_handle, text.c_str(), text.length());
	if (!esc_text) throw std::runtime_error("Can not convert string to URL");

	result = esc_text;
	curl_free(esc_text);

	return result;
}
//----------------------------------------------------------------------------
static size_t write_data(char* ptr, size_t size, size_t nmemb, std::string* data)
{
	if (data)
	{
		data->append(ptr, size * nmemb);
		return size * nmemb;
	}
	else return 0;  // ����� ������
}
//----------------------------------------------------------------------------
static size_t write_head(char* ptr, size_t size, size_t nmemb, std::ostream* stream)
{
	(*stream) << std::string(ptr, size * nmemb);
	return size * nmemb;
}
//---------------------------------------------------------------------------
int main()
{
	/* ���������������� ������ */
	const std::string url_dologin = "http://www.cyberforum.ru/login.php?do=login";// �������� �����������
	const std::string url_user = "http://www.cyberforum.ru/usercp.php"; // ��� �������

	std::string user_name;  //  e-mail
	std::string password;   //  ������

	std::cout << "e-mail: ";
	//getline(std::cin, user_name);
	user_name = "maximusvj@gmail.com";
	std::cout << password << "password: ";
	//getline(std::cin, password);
	password = "g9dEWUfL";
	std::cout << std::endl;

	CURL* curl_handle = curl_easy_init();
	if (curl_handle)
	{
		/* ������������  ������� �� ������ ���������������� ������ */
		std::string post_data;
		post_data += "vb_login_username=" + escape(curl_handle, user_name);
		post_data += "&cookieuser=1";
		post_data += "&vb_login_password=" + escape(curl_handle, password);
		post_data += "&s=&securitytoken=guest";
		post_data += "&do=login";
		post_data += "&vb_login_md5password=";
		post_data += "&vb_login_md5password_utf=";

		curl_easy_setopt(curl_handle, CURLOPT_URL, url_dologin.c_str());

		///  ��������� html ��� c������� � ������ content
		std::string content;
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &content);

		// �������� ������ ������� ������� � �������
		curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, write_head);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, &std::cout);

		// ���� ���������������
		curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
		// max 5-�� ���������������
		curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, 5);
		// ��������� ������������ ����
		curl_easy_setopt(curl_handle, CURLOPT_COOKIEFILE, "");

		/* POST- ������ c ������������ ( ���������� ��������� ������� ) */
		curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, post_data.c_str());
		curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, post_data.length());

		CURLcode res = curl_easy_perform(curl_handle);
		fstream File("file.txt", ios_base::out);
		File << content;
		if (res != CURLE_OK)
		{
			std::cout << curl_easy_strerror(res) << std::endl;
			getchar();
			return 0;
		}
		// ��������� ������� �� ����������������
		if (content.find("<p><strong>�������, ��� �����, Max_Advanced.</strong></p>") != std::string::npos)
			std::cout << "Authenticated!" << std::endl << std::endl;
		else
		{
			std::cout << "Non-Authenticated!" << std::endl;
			curl_easy_cleanup(curl_handle);
			getchar();
			return 0;
		}
		/* GET- ������ ��� �������� � "��� �������" ������ */
		content.clear();
		// ������ POST-����� �� GET
		curl_easy_setopt(curl_handle, CURLOPT_POST, 0);
		curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);
		// ������ url
		curl_easy_setopt(curl_handle, CURLOPT_URL, url_user.c_str());

		res = curl_easy_perform(curl_handle);
		File << content;
		if (res != CURLE_OK)  std::cout << curl_easy_strerror(res) << std::endl;

		// ��������� �������� �� ��� �������
		if (content.find("�������") != std::string::npos)
			std::cout << """My cabinet"" downloaded." << std::endl
			<< "------------- Content -----------------" << std::endl
			<< content << std::endl;
		else  std::cout << "Is not ""My cabinet"" page" << std::endl;

		curl_easy_cleanup(curl_handle);
		File.close();
	}
	system("pause");
	getchar();
	return 0;
}