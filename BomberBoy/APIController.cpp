#include "APIController.h"
#include "picojson.h"


namespace json = picojson;

CLASS_SOURCE(OAPIController)


OAPIController::OAPIController()
{
}


void OAPIController::OnBegin() 
{
	Super::OnBegin();

	bIsLoaded = TryLoadConfig();
	if (bIsLoaded)
	{
		LOG("Using '%s' as API domain", m_apiDomain.c_str());
		m_httpThread = new std::thread(&OAPIController::HttpLoop, this);
	}
	else
		LOG_ERROR("Failed to load API config..");
}

void OAPIController::OnDestroy() 
{
	Super::OnDestroy();
	if (m_httpThread != nullptr)
	{
		bRunThread = false;
		m_httpThread->join();
		delete m_httpThread;
	}
}

bool OAPIController::TryLoadConfig() 
{
	std::ifstream stream(m_configPath, std::ifstream::in);

	// Check file exists
	if (!stream.good())
		return false;

	json::value value;
	stream >> value;

	// Check file is correct format
	if (stream.fail())
	{
		LOG_ERROR("Error parsing config:\n\t'%s'", json::get_last_error().c_str());
		return false;
	}

	// Check reading object
	if (!value.is<json::object>())
	{
		LOG_ERROR("Error reading config: 'Expects config to be json object'");
		return false;
	}

	json::object& obj = value.get<json::object>();
	json::value rawApi = obj.at("API");
	if (!rawApi.is<json::object>())
	{
		LOG_ERROR("Error reading config: 'Missing API object'");
		return false;
	}

	json::object& api = rawApi.get<json::object>();
	json::value domain = api.at("Domain");
	if(!domain.is<string>())
	{
		LOG_ERROR("Error reading config: 'Missing Domain string in API'");
		return false;
	}

	m_apiDomain = domain.get<string>();
	return true;
}


void OAPIController::HttpLoop() 
{
	bRunThread = true;
	LOG("Launching API Http thread");

	while (bRunThread)
	{
		// Execute any requests which are queued
		if (m_httpQueue.size() != 0)
		{
			HttpTask task = m_httpQueue.front();
			m_httpQueue.pop();

			// Re-establish connection each request to clean up properly
			Http connection(m_apiDomain);
			Http::Response response = connection.sendRequest(task.request);
			if (task.callback)
				task.callback(response);
		}
		else
			sf::sleep(sf::milliseconds(250));
	}

	LOG("Shutting down API Http thread");
}

void OAPIController::LoginUser(const string& account, const string& password, HttpCallback callback) 
{
	json::object payload;
	payload["email"] = json::value(account);
	payload["password"] = json::value(password);

	Http::Request request;
	request.setMethod(Http::Request::Post);
	request.setUri(m_apiBaseUri + "/User/Login");
	request.setField("Content-Type", "application/json");
	request.setBody(((json::value)payload).serialize());

	m_httpQueue.emplace(request, 
		[this, callback](Http::Response response)
		{
			// If login succesful, store that information
			if (response.getStatus() == 200)
			{
				json::value body;
				picojson::parse(body, response.getBody());
				json::object obj = body.get<json::object>();

				bLoggedIn = true;
				m_userId = obj["userId"].get<string>();
				m_sessionId = obj["sessionId"].get<string>();
				m_displayName = obj["displayName"].get<string>();
				LOG("Logged in as '%s' (%s)", m_displayName.c_str(), m_userId.c_str());
			}
			else
			{
				bLoggedIn = false;
				LOG("Login failed (%i)", response.getStatus());
			}


			if (callback)
				callback(response);
		}
	);
}