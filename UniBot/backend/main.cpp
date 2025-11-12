#define CROW_MAIN
#include "../crow/crow.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <cmath>
using namespace std;

struct FAQ
{
    string question;
    string answer;
    vector<string> keywords;
};

vector<FAQ> faqs;
string admin_user, admin_pass;

//Admin writing:
void saveFAQs(const string& filename){
    ofstream file(filename);
    for(const auto& faq: faqs){
        file<<"Q:"<<faq.question<<endl;
        file<<"A:"<<faq.answer<<endl;
        file<<"K:";
        for(const auto& keyword: faq.keywords){
            file<<keyword<<" ";
        }
        file<<"\n";
    }
}

//Read faqs.txt file:
void loadFAQs(const string& filename){
    ifstream file(filename);
    faqs.clear();
    FAQ faq;
    string line;

    while(getline(file, line)){
        if (line.find("Q:")==0){
            faq.question=line.substr(2);
        }
        else if(line.find("A:")==0){
            faq.answer=line.substr(2);
        }
        else if(line.find("K:")==0){
            faq.keywords.clear();
            string keyword;
            istringstream key(line.substr(2));
            while(key>>keyword){
                faq.keywords.push_back(keyword);
            }
            faqs.push_back(faq);
        }
        
    }
    cout<<"Loaded "<<faqs.size()<<" faqs"<<endl;
}

//Admin login info:
void loadLoginInfo(const string& filename){
    ifstream file(filename);
    getline(file, admin_user);
    getline(file, admin_pass);
    cout<<"Loaded admin login info."<<endl;
}

//Helper: Tokenize text into words
vector<string> tokenize(const string& text){
    vector<string> tokens;
    string word;
    for(char c : text){
        if(isalnum(c)){
            word += tolower(c);
        } else if(!word.empty()){
            tokens.push_back(word);
            word.clear();
        }
    }
    if(!word.empty()) tokens.push_back(word);
    return tokens;
}

//Helper: Basic stemming (remove common suffixes)
string stem(const string& word){
    string stemmed = word;
    
    // Remove common suffixes
    if(stemmed.length() > 4){
        if(stemmed.substr(stemmed.length()-3) == "ing") 
            stemmed = stemmed.substr(0, stemmed.length()-3);
        else if(stemmed.substr(stemmed.length()-2) == "ed") 
            stemmed = stemmed.substr(0, stemmed.length()-2);
        else if(stemmed.length() > 2 && stemmed.back() == 's' && stemmed[stemmed.length()-2] != 's') 
            stemmed = stemmed.substr(0, stemmed.length()-1);
        else if(stemmed.substr(stemmed.length()-2) == "ly") 
            stemmed = stemmed.substr(0, stemmed.length()-2);
    }
    
    return stemmed;
}

//Helper: Check if word exists in text (with word boundaries)
bool wordExists(const vector<string>& words, const string& keyword){
    for(const auto& word : words){
        if(word == keyword) return true;
    }
    return false;
}

//Helper: Check if stemmed word exists
bool stemmedWordExists(const vector<string>& words, const string& keyword){
    string stemmedKeyword = stem(keyword);
    for(const auto& word : words){
        if(stem(word) == stemmedKeyword) return true;
    }
    return false;
}

//Improved keyword matching with scoring:
int matchFAQ(const string& user_input){
    const double MIN_SCORE_THRESHOLD = 0.3; // Minimum 30% match required
    
    vector<string> userWords = tokenize(user_input);
    int best_index = -1;
    double best_score = 0.0;
    
    for(int i = 0; i < faqs.size(); i++){
        double score = 0.0;
        int exactMatches = 0;
        int stemmedMatches = 0;
        int partialMatches = 0;
        
        for(const auto& keyword : faqs[i].keywords){
            // Exact word match (highest score)
            if(wordExists(userWords, keyword)){
                exactMatches++;
                score += 3.0;
            }
            // Stemmed match (medium score)
            else if(stemmedWordExists(userWords, keyword)){
                stemmedMatches++;
                score += 2.0;
            }
            // Partial substring match (lowest score)
            else if(user_input.find(keyword) != string::npos){
                partialMatches++;
                score += 0.5;
            }
        }
        
        // Normalize score by number of keywords (percentage match)
        if(!faqs[i].keywords.empty()){
            double matchPercentage = (exactMatches + stemmedMatches * 0.7 + partialMatches * 0.3) / faqs[i].keywords.size();
            score = score * matchPercentage;
            
            // Bonus for matching multiple keywords
            if(exactMatches + stemmedMatches >= 2){
                score *= 1.2;
            }
            
            // Update best match if this is better
            if(score > best_score && matchPercentage >= MIN_SCORE_THRESHOLD){
                best_score = score;
                best_index = i;
            }
        }
    }
    
    return best_index;
}

//Add CORS headers to response:
void addCORS(crow::response& res){
    res.add_header("Access-Control-Allow-Origin", "*");
    res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.add_header("Access-Control-Allow-Headers", "Content-Type");
}

//Main:
int main(){
    loadLoginInfo("backend/data/admin.txt");
    loadFAQs("backend/data/faqs.txt");

    crow::App<crow::CORSHandler> app;
    
    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors.global()
        .origin("*")
        .methods("GET"_method, "POST"_method, "PUT"_method, "DELETE"_method, "OPTIONS"_method)
        .headers("Content-Type", "Authorization");

    CROW_ROUTE(app, "/ask").methods("POST"_method, "OPTIONS"_method)
    ([](const crow::request& req) {
        crow::response res;
        if (req.method == "OPTIONS"_method) {
            res.code = 200;
            addCORS(res);
            return res;
        }
        
        auto body = crow::json::load(req.body);
        if (!body) {
            res = crow::response(400, "Invalid JSON");
            addCORS(res);
            return res;
        }

        string userMsg = body["question"].s();
        transform(userMsg.begin(), userMsg.end(), userMsg.begin(), ::tolower);

        int index = matchFAQ(userMsg);
        crow::json::wvalue result;
        if (index != -1)
            result["answer"] = faqs[index].answer;
        else
            result["answer"] = "Sorry, I couldn't find an answer. Please contact admin at +92 316 7695708";

        res = crow::response{result};
        addCORS(res);
        return res;
    });

    CROW_ROUTE(app, "/admin/login").methods("POST"_method, "OPTIONS"_method)
    ([](const crow::request& req) {
        crow::response res;
        if (req.method == "OPTIONS"_method) {
            res.code = 200;
            addCORS(res);
            return res;
        }
        
        auto body = crow::json::load(req.body);
        if (!body) {
            res = crow::response(400, "Invalid JSON");
            addCORS(res);
            return res;
        }

        string username = body["username"].s();
        string password = body["password"].s();

        if (username == admin_user && password == admin_pass)
            res = crow::response{200};
        else
            res = crow::response(401, "Unauthorized");
        
        addCORS(res);
        return res;
    });

    CROW_ROUTE(app, "/admin/faqs").methods("GET"_method, "POST"_method, "OPTIONS"_method)
    ([](const crow::request& req) {
        crow::response response;
        
        if (req.method == "OPTIONS"_method) {
            response.code = 200;
            addCORS(response);
            return response;
        }
        
        if (req.method == "GET"_method) {
            crow::json::wvalue res;
            for (int i = 0; i < faqs.size(); ++i) {
                res[i]["question"] = faqs[i].question;
                res[i]["answer"] = faqs[i].answer;
                res[i]["keywords"] = faqs[i].keywords;
            }
            response = crow::response{res};
            addCORS(response);
            return response;
        }
        
        if (req.method == "POST"_method) {
            auto body = crow::json::load(req.body);
            if (!body) {
                response = crow::response(400, "Invalid JSON");
                addCORS(response);
                return response;
            }

            FAQ newFAQ;
            newFAQ.question = body["question"].s();
            newFAQ.answer = body["answer"].s();
            for (auto& kw : body["keywords"]) newFAQ.keywords.push_back(kw.s());

            faqs.push_back(newFAQ);
            saveFAQs("backend/data/faqs.txt");

            response = crow::response(201);
            addCORS(response);
            return response;
        }
        
        response = crow::response(405, "Method Not Allowed");
        addCORS(response);
        return response;
    });

    CROW_ROUTE(app, "/admin/faqs/<int>").methods("PUT"_method, "DELETE"_method, "OPTIONS"_method)
    ([](const crow::request& req, int index) {
        crow::response res;
        
        if (req.method == "OPTIONS"_method) {
            res.code = 200;
            addCORS(res);
            return res;
        }
        
        if (index < 0 || index >= faqs.size()) {
            res = crow::response(404);
            addCORS(res);
            return res;
        }

        if (req.method == "PUT"_method) {
            auto body = crow::json::load(req.body);
            if (!body) {
                res = crow::response(400);
                addCORS(res);
                return res;
            }

            faqs[index].question = body["question"].s();
            faqs[index].answer = body["answer"].s();
            faqs[index].keywords.clear();
            for (auto& kw : body["keywords"]) faqs[index].keywords.push_back(kw.s());

            saveFAQs("backend/data/faqs.txt");
            res = crow::response(200);
            addCORS(res);
            return res;
        }
        
        if (req.method == "DELETE"_method) {
            faqs.erase(faqs.begin() + index);
            saveFAQs("backend/data/faqs.txt");
            res = crow::response(200);
            addCORS(res);
            return res;
        }
        
        res = crow::response(405, "Method Not Allowed");
        addCORS(res);
        return res;
    });

    app.loglevel(crow::LogLevel::Info)
       .port(18080)
       .bindaddr("127.0.0.1")
       .multithreaded()
       .run();
}