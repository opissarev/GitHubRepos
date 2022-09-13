// GitHubRepos.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <string>
#include <json.hpp>
#include "CUrlRest.h"

using json = nlohmann::json;
using namespace std;

// Returns "true" if JSON property found.
inline bool isJSONProperty(string key, json props)
{
    return props.find(key) != props.end();
}

// Save returns a JSON value as string.
// Returns - JSON value as string or empty string - if property not found 
// or JSON value has "null"
string getJSONValue(string key, json props)
{
    if (isJSONProperty(key, props))
    {
        json value = props.at(key);
        return !value.empty() ? value.get<string>() : "";
    }
    return "";
}

// Displays frendly GitHub error
void showGitHubError(string description, json props)
{
    cout << description;
    if (isJSONProperty("message", props))
    {
        cout << " - " << getJSONValue("message", props) << endl;
    }
}

int main(int argc, char* argv[])
{
    string username;
    cout << "GitHub username:";
    cin >> username;

    CUrlRest rest("https://api.github.com/users/");

    // Check user
    // Sends rest as "curl -X GET https://api.github.com/users/microsoft"
    CURLcode ret = rest.Send(username, "");
    if (ret == CURLE_OK && rest.GetData())
    {
        json output = json::parse(rest.GetData());

        // Check that I got user info
        if (!isJSONProperty("login", output) || username != getJSONValue("login", output))
        {
            showGitHubError("GitHub username is not valid", output);
            return -1;
        }
    }
    else
    {
        cout << "CURL Error - " << ret;
        return -2;
    }

    cout << "Username is valid" << endl;
    cout << "=================================" << endl;

    // Get all repos for user
    // GitHub returns results in portions when it is over 30 (max 100 per page)
    int page = 1, total = 0, totalInPage;
    const int page_limit = 100;
    map<string, int> summary; // <year, count>
    
    do
    {
        // Sends rest as curl -X GET "https://api.github.com/users/microsoft/repos"
        // (sorted by creation date and output by page)
        string arg = "/repos?sort=created_at&direction=asc&page=" + to_string(page)
             + "&per_page=" + to_string(page_limit);
        
        totalInPage = 0;

        ret = rest.Send(username, arg);
        if (ret == CURLE_OK && rest.GetData())
        {
            json output = json::parse(rest.GetData());

            // Check valid response
            if (!output.is_array())
            {
                showGitHubError("GitHub request is not valid", output);
                return -1;
            }
            // I've JSON array of repos
            for (auto& prop : output.items())
            {
                json item = prop.value();
                // Only public repo is displayed
                if (getJSONValue("visibility", item) == "public")
                {
                    // Displays item info
                    cout << getJSONValue("name", item) << endl;

                    string created = getJSONValue("created_at", item);
                    cout << created << endl;

                    string description = getJSONValue("description", item);
                    cout << (description.empty() ? "<Empty>" : description) << endl;

                    cout << "=================================" << endl;

                    // Bonus features
                    // Get year => "2013-10-24T13:28:14Z"
                    string year = created.substr(0, 4);
                    auto summaryYear = summary.find(year);
                    if (summaryYear != summary.end())
                    {
                        // Found => increment count
                        summaryYear->second++;
                    }
                    else
                    {
                        // Not found => Add new
                        summary.insert(make_pair(year, 1));
                    }
                }
                totalInPage++;
            } // for (items array)
            total += totalInPage;
            page++;
        } // rest command
        else
        {
            cout << "CURL Error - " << ret;
            return -2;
        }

        // Ask user to continue...
        // because GitHub has "API rate limit" and possible to exceeded it
        // for users with huge number of repos (microsoft, google)
        // "https://docs.github.com/rest/overview/resources-in-the-rest-api#rate-limiting"
        if (page_limit == totalInPage)
        {
            cout << endl << "This user has more repositories" << endl
                << "Press any letter to continue or [N]o to break.";
            char choice;
            cin >> choice;
            if (choice == 'n' || choice == 'N')
                break;
            cout << "Continue ..." << endl;
        }
        
    } while (page_limit == totalInPage);

    // Displays summary
    cout << "Summary:" << endl;
    for (auto& it : summary)
    {
        cout << it.first << ":\t" << it.second << endl;
    }
    cout << "Total:\t" << total << endl;
    return 0;
}
