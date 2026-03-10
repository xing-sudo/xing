// 此模块应该包含Util工具类，HttpRequest类：保存解析后的请求信息，HttpResponse类：保存响应信息，HttpContext类：保存上下文信息防止请求不完整。
// Util应该包括文件读写，URL编码和解码，状态码及其描述内容的映射，后缀名获取文件类型，判断是否是目录，判断是否是普通文件，资源路径有效性的判断。
#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <regex>
#include"source.cpp"

std::unordered_map<int, std::string> _statu_msg = {
    {100, "Continue"},
    {101, "Switching Protocol"},
    {102, "Processing"},
    {103, "Early Hints"},
    {200, "OK"},
    {201, "Created"},
    {202, "Accepted"},
    {203, "Non-Authoritative Information"},
    {204, "No Content"},
    {205, "Reset Content"},
    {206, "Partial Content"},
    {207, "Multi-Status"},
    {208, "Already Reported"},
    {226, "IM Used"},
    {300, "Multiple Choice"},
    {301, "Moved Permanently"},
    {302, "Found"},
    {303, "See Other"},
    {304, "Not Modified"},
    {305, "Use Proxy"},
    {306, "unused"},
    {307, "Temporary Redirect"},
    {308, "Permanent Redirect"},
    {400, "Bad Request"},
    {401, "Unauthorized"},
    {402, "Payment Required"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {405, "Method Not Allowed"},
    {406, "Not Acceptable"},
    {407, "Proxy Authentication Required"},
    {408, "Request Timeout"},
    {409, "Conflict"},
    {410, "Gone"},
    {411, "Length Required"},
    {412, "Precondition Failed"},
    {413, "Payload Too Large"},
    {414, "URI Too Long"},
    {415, "Unsupported Media Type"},
    {416, "Range Not Satisfiable"},
    {417, "Expectation Failed"},
    {418, "I'm a teapot"},
    {421, "Misdirected Request"},
    {422, "Unprocessable Entity"},
    {423, "Locked"},
    {424, "Failed Dependency"},
    {425, "Too Early"},
    {426, "Upgrade Required"},
    {428, "Precondition Required"},
    {429, "Too Many Requests"},
    {431, "Request Header Fields Too Large"},
    {451, "Unavailable For Legal Reasons"},
    {501, "Not Implemented"},
    {502, "Bad Gateway"},
    {503, "Service Unavailable"},
    {504, "Gateway Timeout"},
    {505, "HTTP Version Not Supported"},
    {506, "Variant Also Negotiates"},
    {507, "Insufficient Storage"},
    {508, "Loop Detected"},
    {510, "Not Extended"},
    {511, "Network Authentication Required"}};

std::unordered_map<std::string, std::string> _mime_msg = {
    {".aac", "audio/aac"},
    {".abw", "application/x-abiword"},
    {".arc", "application/x-freearc"},
    {".avi", "video/x-msvideo"},
    {".azw", "application/vnd.amazon.ebook"},
    {".bin", "application/octet-stream"},
    {".bmp", "image/bmp"},
    {".bz", "application/x-bzip"},
    {".bz2", "application/x-bzip2"},
    {".csh", "application/x-csh"},
    {".css", "text/css"},
    {".csv", "text/csv"},
    {".doc", "application/msword"},
    {".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
    {".eot", "application/vnd.ms-fontobject"},
    {".epub", "application/epub+zip"},
    {".gif", "image/gif"},
    {".htm", "text/html"},
    {".html", "text/html"},
    {".ico", "image/vnd.microsoft.icon"},
    {".ics", "text/calendar"},
    {".jar", "application/java-archive"},
    {".jpeg", "image/jpeg"},
    {".jpg", "image/jpeg"},
    {".js", "text/javascript"},
    {".json", "application/json"},
    {".jsonld", "application/ld+json"},
    {".mid", "audio/midi"},
    {".midi", "audio/x-midi"},
    {".mjs", "text/javascript"},
    {".mp3", "audio/mpeg"},
    {".mpeg", "video/mpeg"},
    {".mpkg", "application/vnd.apple.installer+xml"},
    {".odp", "application/vnd.oasis.opendocument.presentation"},
    {".ods", "application/vnd.oasis.opendocument.spreadsheet"},
    {".odt", "application/vnd.oasis.opendocument.text"},
    {".oga", "audio/ogg"},
    {".ogv", "video/ogg"},
    {".ogx", "application/ogg"},
    {".otf", "font/otf"},
    {".png", "image/png"},
    {".pdf", "application/pdf"},
    {".ppt", "application/vnd.ms-powerpoint"},
    {".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
    {".rar", "application/x-rar-compressed"},
    {".rtf", "application/rtf"},
    {".sh", "application/x-sh"},
    {".svg", "image/svg+xml"},
    {".swf", "application/x-shockwave-flash"},
    {".tar", "application/x-tar"},
    {".tif", "image/tiff"},
    {".tiff", "image/tiff"},
    {".ttf", "font/ttf"},
    {".txt", "text/plain"},
    {".vsd", "application/vnd.visio"},
    {".wav", "audio/wav"},
    {".weba", "audio/webm"},
    {".webm", "video/webm"},
    {".webp", "image/webp"},
    {".woff", "font/woff"},
    {".woff2", "font/woff2"},
    {".xhtml", "application/xhtml+xml"},
    {".xls", "application/vnd.ms-excel"},
    {".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
    {".xml", "application/xml"},
    {".xul", "application/vnd.mozilla.xul+xml"},
    {".zip", "application/zip"},
    {".3gp", "video/3gpp"},
    {".3g2", "video/3gpp2"},
    {".7z", "application/x-7z-compressed"}};

class Util
{
public:
    static bool ReadFile(const std::string &filename, std::string *buff)
    {
        // 读取文件内容到buff
        std::ifstream ifs(filename, std::ios::binary);
        if (ifs.is_open() == false)
        {
            printf("open %s failed\n", filename.c_str());
            return false;
        }
        size_t fsize = 0;
        ifs.seekg(0, ifs.end); // 跳转到文件末尾
        fsize = ifs.tellg();   // 获取文件起始位置相对于当前位置的偏移量
        ifs.seekg(0, ifs.beg); // 跳转到开头
        buff->resize(fsize);
        ifs.read(&(*buff)[0], fsize);
        if (ifs.good() == false)
        {
            printf("read %s failed\n", filename.c_str());
            ifs.close();
            return false;
        }
        ifs.close();
        return true;
    }
   static bool WriteFile(const std::string &filename, const std::string &buff)
    {
        // 将buff写入文件
        std::ofstream ofs(filename, std::ios::binary | std::ios::trunc);
        if (ofs.is_open() == false)
        {
            printf("open %s failed\n", filename.c_str());
            return false;
        }
        ofs.write(buff.c_str(), buff.size());
        if (ofs.good() == false)
        {
            printf("write %s failed\n", filename.c_str());
            ofs.close();
            return false;
        }
        ofs.close();
        return true;
    }
    // URL编码，避免URL中资源路径与查询字符串中的特殊字符与HTTP请求中特殊字符产生歧义
    // 编码格式：将特殊字符的ascii值，转换为两个16进制字符，前缀%   C++ -> C%2B%2B
    //   不编码的特殊字符： RFC3986文档规定 . - _ ~ 字母，数字属于绝对不编码字符
    // RFC3986文档规定，编码格式 %HH
    // W3C标准中规定，查询字符串中的空格，需要编码为+， 解码则是+转空格
    static std::string UrlEncode(const std::string &url, bool convert)
    {
        std::string res;
        for (auto &c : url)
        {
            // 当字符为._-~ 普通数字和字母时追加 isalnum（）用于判断字符是否是字母或者数字
            if (c == '.' || c == '_' || c == '~' || c == '-' || isalnum(c))
            {
                res += c;
                continue;
            }
            if (c == ' ' && convert == true)
            {
                res += '+';
                continue;
            }
            // 其余字符要编码成%HH形式，两位十六进制数
            char tmp[4] = {0};
            snprintf(tmp, 4, "%%%02X", c); //%02X表示以2位十六进制数表示
            res += tmp;
        }
        return res;
    }
    static char HEXTOI(char c)
    {
        // 将16进制字符转换为数字
        if (c >= '0' && c <= '9')
        {
            return c - '0';
        }
        else if (c >= 'a' && c <= 'z')
        {
            return c - 'a' + 10; //+10是因为a~f对应10~15
        }
        else if (c >= 'A' && c <= 'Z')
        {
            return c - 'A' + 10;
        }
        return -1;
    }
    static std::string UrlDecode(const std::string &url, bool convert)
    {
        // 遇到%，则将紧随其后的两个字符转换为数字第一个左移4位+第二个
        std::string res;
        for (int i = 0; i < url.size(); i++)
        {
            if (url[i] == '+' && convert == true)
            {
                res += ' ';
                continue;
            }
            if (url[i] == '%' && (i + 2) < url.size())
            {
                char v1 = HEXTOI(url[i + 1]);
                char v2 = HEXTOI(url[i + 2]);
                char v = v1 * 16 + v2;
                res += v;
                i += 2;
                continue;
            }
            res += url[i];
        }
        return res;
    }

    static std::string StatusDesc(int status)
    {
        auto it = _statu_msg.find(status);
        if (it != _statu_msg.end())
        {
            return it->second;
        }
        return "Unknow";
    }
    static std::string ExtMime(const std::string &filename)
    {
        // 获取后缀再分割子串
        size_t pos = filename.find_last_of('.');
        if (pos == std::string::npos)
        {
            return "application/octet-stream";
        }
        std::string ext = filename.substr(pos);
        auto it = _mime_msg.find(ext);
        if (it == _mime_msg.end())
        {
            return "application/octet-stream";
        }
        return it->second;
    }
    static bool IsDir(const std::string &filename)
    {
        struct stat st;
        int ret = stat(filename.c_str(), &st);
        if (ret < 0)
        {
            return false;
        }
        return S_ISDIR(st.st_mode);
    }
   static bool Isregular(const std::string &filename)
    {
        struct stat st;
        int ret = stat(filename.c_str(), &st);
        if (ret < 0)
        {
            return false;
        }
        return S_ISREG(st.st_mode);
    }
   static bool ValidPath(const std::string &path)
    {
        //../算违规，利用深度进行合理性判断
        std::vector<std::string> subdir;
        Split(path, "/", &subdir);
        int level = 0;
        for (auto &s : subdir)
        {
            if (s == "..")
            {
                level--;
                if (level < 0)
                    return false;
                continue;
            }
            level++;
        }
        return true;
    }
    static size_t Split(const std::string &src, const std::string &sep, std::vector<std::string> *arr)
    {
        // 改函数按照分隔符sep将src分割成数组arr，并返回数组大小
        size_t offset = 0;
        while (offset < src.size())
        {
            size_t pos = src.find(sep, offset);
            if (pos == std::string::npos)
            {
                arr->push_back(src.substr(offset));
                return arr->size();
            }
            if (pos == offset)
            {
                offset = pos + sep.size();
                continue; // 跳过空字符串
            }
            arr->push_back(src.substr(offset, pos - offset));
            offset = pos + sep.size();
        }
        return arr->size();
    }
};

class HttpRequest // 储存HTTP信息要素:请求方法，请求路径，请求版本，请求头部，请求内容，content-length,长短连接
{
public:
    std::string _method;                                   // 请求方法
    std::string _path;                                     // 请求路径
    std::string _verson;                                   // 版本
    std::string _body;                                     // 正文
    std::smatch _matches;                                  // 资源路径的正则提取数据
    std::unordered_map<std::string, std::string> _headers; // 头部
    std::unordered_map<std::string, std::string> _params;  // 查询字符串
public:
    HttpRequest() : _verson("HTTP/1.1") {}
    void Reset()
    {
        _method.clear();
        _path.clear();
        _verson = "HTTP/1.1";
        _body.clear();
        std::smatch match;
        _matches.swap(match);
        _headers.clear();
        _params.clear();
    }
    // 插入头部字段
    void SetHeader(const std::string &key, const std::string &value)
    {
        _headers.insert(std::make_pair(key, value));
    }
    // 判断是否存在对应的字段
    bool Hasheader(const std::string &key) const
    {
        auto it = _headers.find(key);
        if (it != _headers.end())
        {
            return true;
        }
        return false;
    }
    // 获取字段对应的值
    std::string GetHeader(const std::string &key) const
    {
        auto it = _headers.find(key);
        if (it != _headers.end())
        {
            return it->second;
        }
        return "";
    }
    // 插入查询字符串
    void SetParam(const std::string &key, const std::string &value)
    {
        _params.insert(std::make_pair(key, value));
    }
    // 判断是否存在对应的查询字符串
    bool HasParam(const std::string &key) const
    {
        auto it = _params.find(key);
        if (it != _params.end())
        {
            return true;
        }
        return false;
    }
    // 获取查询字符串对应的值
    std::string GetParam(const std::string &key) const
    {
        auto it = _params.find(key);
        if (it != _params.end())
        {
            return it->second;
        }
        return "";
    }
    // 获取正文长度
    size_t ContentLength() const
    {
        // 获取Content-Length字段
        bool ret = Hasheader("Content-Length");
        if (ret == false)
        {
            return 0;
        }
        std::string len = GetHeader("Content-Length");
        return std::stol(len);
    }
    // 判断是否是短链接
    bool Close() const
    {
        // 没有connection字段或者值为close都是短链接
        if (Hasheader("Connection") == true && GetHeader("Connection") == "keep-alive")
        {
            return false;
        }
        return true;
    }
};

class HttpResponse // 储存HTTP响应信息要素:状态码，响应头部，响应内容,重定向
{
public:
    int _status;                                           // 状态码
    std::string _body;                                     // 响应正文
    bool _redirect_flag;                                   // 是否重定向
    std::string _redirect_url;                             // 重定向地址
    std::unordered_map<std::string, std::string> _headers; // 响应头部
public:
    void Reset()
    {
        _status = 200;
        _redirect_flag = false;
        _body.clear();
        _redirect_url.clear();
        _headers.clear();
    }
    // 插入头部字段
    void SetHeader(const std::string &key, const std::string &value)
    {
        _headers.insert(std::make_pair(key, value));
    }
    // 判断是否存在对应的字段
    bool Hasheader(const std::string &key) const
    {
        auto it = _headers.find(key);
        if (it != _headers.end())
        {
            return true;
        }
        return false;
    }
    // 获取指定字段的值
    std::string GetHeader(const std::string &key) const
    {
        auto it = _headers.find(key);
        if (it != _headers.end())
        {
            return it->second;
        }
        return "";
    }
    // 设置正文
    void Setcontent(const std::string &body, const std::string &type = "text/html")
    {
        _body = body;
        SetHeader("Content-Type", type);
    }
    // 设置重定向
    void SetRedirect(const std::string &url, int status = 302)
    {
        _status = status;
        _redirect_flag = true;
        _redirect_url = url;
    }
    bool Close()
    {
        if (Hasheader("Connection") == true && GetHeader("Connection") == "keep-alive")
        {
            return false;
        }
        return true;
    }
};

typedef enum
{
    RECV_HTTP_ERROR,
    RECV_HTTP_LINE,
    RECV_HTTP_HEAD,
    RECV_HTTP_BODY,
    RECV_HTTP_OVER
} HttpRecvStatus;

#define MAX_LINE 8192 // 最大行长度
class HttpContext // 请求更新上下文，接收的数据不是一条完整的数据
{
private:
    int _resp_status;            // 响应状态码
    HttpRecvStatus _recv_status; // 接收及解析状态
    HttpRequest _request;        // 已经解析得到的请求信息
private:
    bool RecvHttpLine(Buffer *buff)
    {
        if(_recv_status!=RECV_HTTP_LINE)
        {
            return false;
        }
        std::string line=buff->Getline();
        if(line.size()==0)
        {   //如果buff中的可读数据比最大长度还大但还是不足一行，那一定是有问题的
            if(buff->ReadAbleSize()>MAX_LINE)
            {
                _recv_status=RECV_HTTP_ERROR;
                _resp_status=414;//URI Too Long
                return false;
            }
            //数据不够再等等
            return true;
        }
        if(line.size()>MAX_LINE)
        {
            _recv_status=RECV_HTTP_ERROR;
            _resp_status=414;//URI Too Long
            return false;
        }
        bool ret=ParseHttpLine(line);
        if(ret==false)
        {
            return false;
        }
        //首行处理完毕，进入头部处理
        _recv_status=RECV_HTTP_HEAD;
        return true;
     }
    bool RecvHttpHead(Buffer *buff)
    {
        if(_recv_status!=RECV_HTTP_HEAD)
        {
            return false;
        }
        while(1)
        {
            std::string line=buff->Getline();
            if(line.size()==0)
            {
                if(buff->ReadAbleSize()>MAX_LINE)
                {
                    _recv_status=RECV_HTTP_ERROR;
                    _resp_status=414;//URI Too Long
                    return false;
                }
                return true;
                
            }
            if(line.size()>MAX_LINE)//数据太长处理不了
            {
                _recv_status=RECV_HTTP_ERROR;
                _resp_status=414;//URI Too Long
                return false;
            }
            if(line=="\n"||line=="\r\n")//读到空行，头部结束
            {
                break;
            }
             bool ret=ParseHttpHead(line);
             if(ret==false)
             {
                return false;
             }
        }
        _recv_status=RECV_HTTP_BODY;
        return true;
    }
    bool RecvHttpBody(Buffer *buff)
    {
        if(_recv_status!=RECV_HTTP_BODY)
        {
            return false;
        }
        //获取正文长度
        size_t len=_request.ContentLength();
        if(len==0)
        {
            _recv_status=RECV_HTTP_OVER;
            return true;
        }
        //获取实际需要读取的长度
        int real_len=len-_request._body.size();
        //缓冲区内的数据很多，就读完
        if(buff->ReadAbleSize()>= real_len)
        {
            _request._body.append(buff->GetReadPos(),real_len);
            buff->MoveReadPos(real_len);
            _recv_status=RECV_HTTP_OVER;
            return true;
        }
        //缓冲区的数据不满足需要读取的长度，就有多少读多少，等待下次
        _request._body.append(buff->GetReadPos(),buff->ReadAbleSize());
        buff->MoveReadPos(buff->ReadAbleSize());
        return true;
    }   
    bool ParseHttpLine(const std::string& line)
    {   //正则匹配
        std::smatch matches;
        std::regex e("(GET|POST|PUT|HEAD|DELETE)([^?]*)(?:\\?(.*))?(HTTP/1\\.[01])(?:\n|\r\n)?",std::regex::icase);
        bool ret= std::regex_match(line,matches,e);
        if(ret==false)
        {
            _recv_status=RECV_HTTP_ERROR;
            _resp_status=400;//Bad Request
            return false;
        }
        _request._method=matches[1];//请求方法
        _request._path=Util::UrlDecode(matches[2],false);//请求路径
        _request._verson=matches[4];//版本
        //提取查询字符串
        std::vector<std::string> query_string_arr;
        std::string query_str=matches[3];
        //查询字符串的格式是key1=value1&key2=value2按&将其分割
        Util::Split(query_str,"&",&query_string_arr);
        //针对各个子串 以 = 分割得到key和value 进行解码后插入
        for(auto& str:query_string_arr)
        {
            size_t pos=str.find("=");
            if(pos==std::string::npos)
            {
                _recv_status=RECV_HTTP_ERROR;
                _resp_status=400;//Bad Request
                return false;
            }
            std::string key=Util::UrlDecode(str.substr(0,pos),true);
            std::string value=Util::UrlDecode(str.substr(pos+1),true);
            _request.SetParam(key,value);
        }
        return true;
    }
    bool ParseHttpHead( std::string& head)
    {
        //末尾是\r或\n去掉
        if(head.back()=='\r')
        {  
            head.pop_back();
        }
        if(head.back()=='\n')
        {
            head.pop_back();
        }
        size_t pos=head.find(": ");
        if(pos==std::string::npos)
        {
            _recv_status=RECV_HTTP_ERROR;
            _resp_status=400;//Bad Request
            return false;
        }
        std::string key=head.substr(0,pos);
        std::string value=head.substr(pos+2);
        _request.SetHeader(key,value);
        return true;
    }
public:
    HttpContext():_resp_status(200),_recv_status(RECV_HTTP_LINE){}
    void Reset()
    {
        _resp_status=200;
        _recv_status=RECV_HTTP_LINE;
        _request.Reset();
    }
    int GetRespStatus()
    {
        return _resp_status;
    }
    HttpRecvStatus GetRecvStatus()
    {
        return _recv_status;
    }
    HttpRequest& GetRequest()
    {
        return _request;
    }
    void RecvHttpRequest(Buffer* buff)
    {
        switch(_recv_status)
        {
            //不同状态处理不同的事，但不能break，因为处理完头部应该立刻跳转处理正文
            case RECV_HTTP_LINE:RecvHttpLine(buff);
            case RECV_HTTP_HEAD:RecvHttpHead(buff);
            case RECV_HTTP_BODY:RecvHttpBody(buff);
        }
        return;
    }
};
class HttpServer // 服务器类，负责监听端口，接收请求，处理请求，返回响应
{};