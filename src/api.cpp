#include <curl/curl.h>
#include "api.h"

namespace VK
{

using json = nlohmann::json;

const std::string Client::api_url       {"https://api.vk.com/method/"};
const std::string Client::app_id        {"3140623"}; // android=2274003
const std::string Client::app_secret    {"VeWdmVclDCtn6ihuP1nt"}; // android=hHbZxrka2uZ6jB1inYsH
const std::string Client::scope         {"offline,groups,messages,friends,audio"};
const std::string Client::auth_url      {"https://oauth.vk.com/token?"};



bool Client::oauth(const callback_func_cap handler)
{
    if (handler == nullptr) {
        return false;
    }
    this->clear();

    std::string oauth_url{"https://oauth.vk.com/authorize?"};
    const params_map params
    {
        {"client_id",       app_id},
        {"display",         "page"},
        {"redirect_uri",    "https://oauth.vk.com/blank.html"},
        {"scope",           scope},
        {"response_type",   "token"},
        {"v",               version},
    };

    oauth_url += Utils::data2str(params);
    const std::string blank{handler(oauth_url)};

    if (blank.empty()) {
        return false;
    }

    auto it{blank.find("=")};
    if (it == std::string::npos) {
        return false;
    }

    it++;
    this->a_t = blank.substr(it);
    it = this->a_t.find("&expires_in");

    if (it == std::string::npos) {
        this->clear();
        return false;
    }

    this->a_t = a_t.substr(0, it);

    return !this->a_t.empty();
}


Client::Client(const std::string _version,
                   const std::string _lang,
                   const VK::callback_func_cap cap_callback,
                   const VK::callback_func_fa2 _fa2_callback)
    : captcha_callback(cap_callback)
    , fa2_callback(_fa2_callback)
    , version(_version)
    , lang(_lang)
{}



bool Client::check_access()
{
    const json jres{call("users.get", "")};

    if (jres.find("error") != jres.end()) {
        this->clear();
        return false;
    }

    try {
        json info{jres.at("response").get<json>()};
        info = info.begin().value();
        user.parse(info);
    }
    catch(...) {
        this->clear();
        return false;
    }

    return true;
}



bool Client::auth(const std::string& login, const std::string& pass, const std::string& access_token)
{
    if (!access_token.empty()) {
        this->a_t = access_token;
        if (check_access()) {
            return true;
        }
    }
    if (login.empty() || pass.empty()) {
        return false;
    }

    this->a_t.clear();

    params_map params {
        {"client_id",       app_id},
        {"grant_type",      "password"},
        {"client_secret",   app_secret},
        {"scope",           scope},
        {"username",        login},
        {"password",        pass},
    };

    if (!captcha_sid.empty()) {
        params.insert({"captcha_sid", captcha_sid});
        params.insert({"captcha_key", captcha_key});
    }
    if (fa2_callback != nullptr) {
        params.insert({"2fa_supported", "1"});
    }
    if (!fa2_code.empty()) {
        params.insert({"code", fa2_code});
    }

    const std::string data{VK::Utils::data2str(params)};
    captcha_sid.clear();
    captcha_key.clear();
    fa2_code.clear();

    const std::string res{request(auth_url, data)};
    if (res.empty()) {
        return false;
    }

    try {
        const json jres{json::parse(res)};

        if (jres.find("error") == jres.end() || jres.find("access_token") != jres.end()) {
            this->a_t = jres.at("access_token").get<std::string>();
            this->user.user_id = jres.at("user_id").get<size_t>();
            return check_access();
        }

        this->l_error = jres.at("error").get<std::string>();

        if (this->l_error == "invalid_client" || this->l_error == "invalid_request") {
            return false;
        }
        else if(this->l_error == "need_validation") {
            fa2_code = get_fa2_code();
            if (!fa2_code.empty()) {
                return this->auth(login, pass);
            }
        }
        else if (this->l_error == "need_captcha") {
            captcha_sid = jres.at("captcha_sid").get<std::string>();
            captcha_key = get_captcha_key(captcha_sid);
            if (!captcha_key.empty()) {
                return this->auth(login, pass);
            }
        }
    }
    catch(...) {}

    return false;
}



json Client::call(const std::string& method, const std::string& params)
{
    if (method.empty()) {
        return nullptr;
    }

    const std::string url{api_url + method};
    std::string data{params + ((params.empty()) ? "" : "&")};

    params_map tmp_params{};

    if (!captcha_sid.empty()) {
        tmp_params.insert({"captcha_sid", captcha_sid});
        tmp_params.insert({"captcha_key", captcha_key});
    }
    tmp_params.insert({"v", version});
    tmp_params.insert({"lang", lang});
    
    if (!a_t.empty()) {
        tmp_params.insert({"access_token", a_t});
    }

    data += VK::Utils::data2str(tmp_params);
    captcha_sid.clear();
    captcha_key.clear();

    const std::string res{request(url, data)};
    if (res.empty()) {
        return nullptr;
    }

    try {
        const json jres{json::parse(res)};

        if (jres.find("error") == jres.end()) {
            return jres;
        }

        const json item{jres.at("error").get<json>()};
        this->l_error = item.at("error_msg").get<std::string>();

        if (this->l_error == "need_captcha") {
            captcha_sid = item.at("captcha_sid").get<std::string>();
            captcha_key = get_captcha_key(captcha_sid);
            if (!captcha_key.empty()) {
                return this->call(method, params);
            }
        }

        return jres;
    }
    catch(...) {}

    return nullptr;
}



void Client::clear()
{
    a_t.clear();
    user.first_name.clear();
    user.last_name.clear();
    user.user_id = 0;

    captcha_sid.clear();
    captcha_key.clear();
    fa2_code.clear();
}



json Client::call(const std::string& method, const params_map& params)
{
    if (method.empty()) {
        return nullptr;
    }

    std::string data{};
    if (params.size()) {
        data = VK::Utils::data2str(params);
    }

    return this->call(method, data);
}



std::string Utils::char2hex(const char dec)
{
    char dig1 = (dec & 0xF0) >> 4;
    char dig2 = (dec & 0x0F);

    if (0  <= dig1 && dig1 <=  9) dig1 += 48;
    if (10 <= dig1 && dig1 <= 15) dig1 += 87;
    if (0  <= dig2 && dig2 <=  9) dig2 += 48;
    if (10 <= dig2 && dig2 <= 15) dig2 += 87;

    std::string ret{std::to_string(dig1)};
    ret += dig2;

    return ret;
}



std::string Client::request(const std::string& url, const std::string& data)
{
    static char errorBuffer[CURL_ERROR_SIZE];
    curl_buffer.clear();

    CURL* curl{curl_easy_init()};
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER,     errorBuffer);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER,  0L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT,       "VK API Client");
        curl_easy_setopt(curl, CURLOPT_URL,             url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS,      data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,   Utils::CURL_WRITER);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA,       &curl_buffer);

        const CURLcode result{curl_easy_perform(curl)};
        curl_easy_cleanup(curl);
        if (result == CURLE_OK) {
            return curl_buffer;
        }
        else {
            return errorBuffer;
        }
    }
    curl_easy_cleanup(curl);

    return "";
}



std::string Utils::urlencode(const std::string& url)
{
    std::string escaped{};
    for (const char c : url) {
        if ((48 <= c && c <= 57) || (65 <= c && c <= 90)  || (97 <= c && c <= 122) ||
            (c =='~' || c =='!' || c =='*' || c =='(' || c ==')' || c =='\''))
        {
            escaped += c;
        }
        else {
            escaped += '%';
            escaped += char2hex(c);
        }
    }
    return escaped;
}



std::string Utils::data2str(const params_map& data)
{
    std::string result{};
    for(const auto& kv : data) {
        result += kv.first + "=" + urlencode(kv.second) + "&";
    }

    return result;
}



int Utils::CURL_WRITER(char* data, size_t size, size_t nmemb, std::string* buffer)
{
    int result = 0;
    if (buffer != NULL) {
        buffer->append(data, size * nmemb);
        result = size * nmemb;
    }
    return result;
}

} // namespace VK
