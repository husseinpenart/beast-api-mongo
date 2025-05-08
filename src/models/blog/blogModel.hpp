#include <string>
#include <bsoncxx/document/value.hpp>
#include <bsoncxx/document/view.hpp>
using namespace std;
class blogModel
{
public:
    string id;
    string title;
    string category;
    string shortDesc;
    string description;
    string image;
    // constructors
    blogModel(const string &title, const string &category, const string &shortDesc, const string &description, const string &image );

    // methods
    bsoncxx::document::value toBson() const;
    static blogModel fromBson(const bsoncxx::document::view &doc);
};


