#include "attachment.h"
#include <array>

namespace VK
{
const std::string Attachment::Audio::type       {"audio"};
const std::string Attachment::Photo::type       {"photo"};
const std::string Attachment::Document::type    {"doc"};



bool Attachment::BaseAttachment::parse_type(const json& data)
{
    try {
        this->parsed_type = data.at("type").get<std::string>();
        return true;
    }
    catch(...) {
        this->parsed_type.clear();
    }
    return false;
}



bool Attachment::BaseAttachment::parse_common(const json& data)
{
    try {
        this->date = data.at("date").get<size_t>();
        this->id = data.at("id").get<int>();
        this->owner_id = data.at("owner_id").get<int>();
        return true;
    }
    catch(...) {}

    return false;
}



bool Attachment::Audio::parse(const json& data)
{
    if (data == nullptr) {
        return false;
    }

    try {
        if (!parse_type(data) || parsed_type != type) {
            return false;
        }

        const json att = data.at(type).get<json>();
        if (att == nullptr) {
            return false;
        }
    
        parse_common(att);
        this->title = att.at("title").get<std::string>();
        this->artist = att.at("artist").get<std::string>();
        this->duration = att.at("duration").get<size_t>();
        this->direct_url = att.at("url").get<std::string>();
    
        return true;
    }
    catch(...) {}

    return false;
}



bool Attachment::Photo::parse(const json& data)
{
    if (data == nullptr) {
        return false;
    }

    const std::array<std::string, 6> sizes{"photo_2560", "photo_1280", "photo_807", "photo_604", "photo_130", "photo_75"};

    try {
        if (!parse_type(data) || parsed_type != type) {
            return false;
        }

        const json att = data.at(type).get<json>();
        if (att == nullptr) {
            return false;
        }
    
        parse_common(att);
        this->text = att.at("text").get<std::string>();

        for (const auto& size: sizes) {
            if (att.find(size) != att.end()) {
                this->direct_url = att.at(size).get<std::string>();
                break;
            }
        }
    
        return true;
    }
    catch(...) {}

    return false;
}



std::string Attachment::Document::doc_type_str(const int tp)
{
    switch (tp) {
    case 1:     return "text";
    case 2:     return "archive";
    case 3:     return "gif";
    case 4:     return "image";
    case 5:     return "audio";
    case 6:     return "video";
    case 7:     return "book";
    default:    return "unknown";
    }
}



bool Attachment::Document::parse(const json &data)
{
    if (data == nullptr) {
        return false;
    }

    try {
        if (!parse_type(data) || parsed_type != type) {
            return false;
        }

        json att = data.at(type).get<json>();
        if (att == nullptr) {
            return false;
        }
    
        parse_common(att);
        this->title = att.at("title").get<std::string>();
        this->ext = att.at("ext").get<std::string>();
        this->doc_type = att.at("type").get<int>();
        this->size = att.at("size").get<size_t>();
        this->direct_url = att.at("url").get<std::string>();
        return true;
    }
    catch(...) {}

    return false;
}



bool Attachment::User::parse(const VK::json& data)
{
    if (data == nullptr) {
        return false;
    }

    try {
        this->first_name = data.at("first_name").get<std::string>();
        this->last_name = data.at("last_name").get<std::string>();
        this->user_id = data.at("id").get<size_t>();
        return true;
    }
    catch(...) {}

    return false;
}

} // namespace VK
