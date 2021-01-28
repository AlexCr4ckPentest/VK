#ifndef VK_ATTACHMENT_H
#define VK_ATTACHMENT_H

#include "json.hpp"

namespace VK
{

using json = nlohmann::json;

namespace Attachment
{

/* class for store data
 */
class DataModel
{
public:
    /* parse json and fill self fileds
     * if all is ok returned true
     */
    virtual bool parse(const json& data) = 0;

    /* return class fields in std::string
     */
    virtual std::string dump() const = 0;

    inline bool is_parsed() const noexcept
    { return parsed; }

    virtual ~DataModel() {}

protected:
    bool parsed;
};



/* VK User Info
 */
class User : public DataModel
{
public:
    std::string first_name;
    std::string last_name;
    size_t user_id;

    bool parse(const json& data);

    inline std::string dump() const override
    { return std::to_string(user_id) + " - " + first_name + " " + last_name; }

    virtual ~User() {}
};



/* class for store common data of VK Attacment
 */
class BaseAttachment : public DataModel
{
public:
    int id;
    int owner_id;

    size_t date; /* timestamp date attachment */
    std::string direct_url; /* url to download attachment */

    virtual ~BaseAttachment() {}

protected:
    std::string parsed_type;
    bool parse_type(const json& data);
    bool parse_common(const json& data);
};



/* VK Audio Attachment
 */
class Audio : public BaseAttachment
{
public:
    static const std::string type; /* need to make request to API */
    std::string artist;
    std::string title;
    size_t duration; /* in seconds */

    bool parse(const json& data);

    inline std::string dump() const override
    { return artist + " - " + title + " : " + std::to_string(duration); }

    virtual ~Audio() {}
};



/* VK Photo Attachment
 */
class Photo : public BaseAttachment
{
public:
    static const std::string type; /* need to make request to API */
    std::string text;

    bool parse(const json& data);

    inline std::string dump() const override
    { return text + " - " + direct_url + " : " + std::to_string(date); }

    virtual ~Photo() {}
};



/* VK Document Attachment
 */
class Document : public BaseAttachment
{
public:
    static const std::string type; /* need to make request to API */
    std::string title;
    std::string ext; /* for e.g. mp3, gif, jpg */
    size_t size = 0; /* in byte */
    int doc_type = -1;

    static std::string doc_type_str(const int tp);
    bool parse(const json& data);

    inline std::string dump() const
    { return title + " - " + ext + " : " + std::to_string(size); }

    virtual ~Document() {}
};

} // namespace Attachment
} // namespace VK

#endif // VK_ATTACHMENT_H
