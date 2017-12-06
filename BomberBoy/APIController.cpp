#include "APIController.h"

#define PICOJSON_USE_INT64
#include "picojson.h"

namespace json = picojson;

CLASS_SOURCE(OAPIController)
#define API_TOKEN "5c81377e82d3c32ff464fb3cbcf0ef77"


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

				// Update local controllers
				OBPlayerController* player = GetGame()->GetFirstObject<OBPlayerController>(true);
				if (player != nullptr)
				{
					player->SetPlayerName(m_displayName);
					player->m_userId = m_userId;
				}
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

void OAPIController::VerifyUser(const string& userId, const string& sessionId, HttpCallback callback) 
{
	json::object payload;
	payload["userId"] = json::value(userId);
	payload["sessionId"] = json::value(sessionId);

	Http::Request request;
	request.setMethod(Http::Request::Post);
	request.setUri(m_apiBaseUri + "/User/Auth");
	request.setField("Content-Type", "application/json");
	request.setBody(((json::value)payload).serialize());
	m_httpQueue.emplace(request, callback);
}

void OAPIController::ReportMatchResults(const int64& startEpoch, const std::vector<OBPlayerController*>& players, HttpCallback callback)
{
	json::object payload;
	payload["startTime"] = json::value(startEpoch);

	json::array playerStats;
	for (OBPlayerController* player : players)
	{
		json::object stats;
		stats["userId"] = json::value(player->GetUserID());
		stats["kills"] = json::value((int64)player->GetCharacter()->GetKills());
		stats["deaths"] = json::value((int64)player->GetCharacter()->GetDeaths());
		stats["roundsWon"] = json::value((int64)player->GetCharacter()->GetRoundsWon());
		stats["bombsPlaced"] = json::value((int64)player->GetCharacter()->GetBombsPlaced());
		playerStats.push_back(json::value(stats));
	}
	payload["playerStats"] = json::value(playerStats);


	Http::Request request;
	request.setMethod(Http::Request::Post);
	request.setUri(m_apiBaseUri + "/Match/Result");
	request.setField("Content-Type", "application/json");
	request.setField("api-token", API_TOKEN);
	request.setBody(((json::value)payload).serialize());

	m_httpQueue.emplace(request,
		[this, callback](Http::Response response)
		{
			// If login succesful, store that information
			if (response.getStatus() == 200)
			{
				LOG("Match results sent sucessfully!");
			}
			else
			{
				LOG_WARNING("Failed to send match results: \n%s", response.getBody().c_str());
			}


			if (callback)
				callback(response);
		}
	);
}