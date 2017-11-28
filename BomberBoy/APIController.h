#pragma once
#include "Core\Core-Common.h"

#include <thread>
#include <fstream>
#include <functional>
#include <queue>
#include <SFML\Network.hpp>


// Makes writting just a little bit shorter
typedef sf::Http Http;
typedef std::function<void(Http::Response&)> HttpCallback;


/**
* Handles calls to and from the RESTful API
* If you wish to enable this make sure to define API_SUPPORTED in the preprocessor definitions
*/
class OAPIController : public OObject
{
	CLASS_BODY()
private:
	///
	/// API information
	///
	bool bIsLoaded = false;
	const string m_configPath = "config.json";
	const string m_apiBaseUri = "/BomberBoy/API/v1.0";
	string m_apiDomain;

	///
	/// Threading information
	///
	bool bRunThread = false;
	std::thread* m_httpThread = nullptr;

	struct HttpTask
	{
		Http::Request request;
		HttpCallback  callback;

		HttpTask(Http::Request request, HttpCallback callback) : request(request), callback(callback) {}
	};
	std::queue<HttpTask> m_httpQueue;


	///
	/// User information
	///
	bool bLoggedIn = false;
	string m_displayName;
	string m_userId;
	string m_sessionId;

public:
	OAPIController();


	virtual void OnBegin() override;
	virtual void OnDestroy() override;

	/**
	* Attempt to log in to a user account
	* @param account		The email of the account
	* @param password		The password of the account
	* @param callback		The callback to call when the user succesfully logs in
	*/
	void LoginUser(const string& account, const string& password, HttpCallback callback = HttpCallback());

	/**
	* Logout the currently connected user
	*/
	inline void LogoutUser()
	{
		bLoggedIn = false;
		m_displayName = "";
		m_userId = "";
		m_sessionId = "";
	}

private:
	/** Loop that will get called from another thread to handle http requests */
	void HttpLoop();

	/**
	* Attempt to load settings to use for this controller fron config
	* @returns Was load succesful
	*/
	bool TryLoadConfig();


	/**
	* Getters & Setters
	*/
public:
	inline const string& GetDomain() const { return m_apiDomain; }
	inline const string& GetBaseURI() const { return m_apiBaseUri; }

	inline const bool& IsUserLoggedIn() const { return bLoggedIn; }
	inline const string& GetUserDisplayName() const { return m_displayName; }
	inline const string& GetUserID() const { return m_userId; }
	inline const string& GetUserSessionID() const { return m_sessionId; }
};

