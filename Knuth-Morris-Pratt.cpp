#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <cmath>
#include <iomanip>
using namespace std;
using namespace filesystem;

class KMP {
private:
    int* kmpNext;
    string pattern;

    void preKmp() {
        int i = 0;
        int j = kmpNext[0] = -1;

        while (i < pattern.length()) {
            while (j > -1 && pattern[i] != pattern[j])
                j = kmpNext[j];
            i++;
            j++;
            if (pattern[i] == pattern[j])
                kmpNext[i] = kmpNext[j];
            else
                kmpNext[i] = j;
        }
    }

public:
    explicit KMP(const string& pattern) {
        this->pattern = pattern;
        kmpNext = new int[pattern.length()];

        // Preprocessing
        preKmp();
    };

    long execute(string searchIn, bool caseSensitive = false) {
        int i, j;

        // Searching
        i = j = 0;
        // remember the indices that match
        long count = 0;

        while (j < searchIn.length()) {
            while (i > -1 && !compare(pattern[i], searchIn[j], caseSensitive))
                i = kmpNext[i];
            i++;
            j++;
            if (i >= pattern.length()) {
                count++;
                i = kmpNext[i];
            }
        }

        return count;
    }

    static bool compare(char x, char y, bool caseSensitive = false) {
        if( !caseSensitive && ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z')) &&
                ((y >= 'a' && y <= 'z') || (y >= 'A' && y <= 'Z'))) {
            return abs(x - y) == 0 || abs(x - y) == abs('A' - 'a');
        } else {
            return x == y;
        }
    }
};

class IO {
public:
    static string readFile(const string& path) {
        ifstream fin(path);

        string content((istreambuf_iterator<char>(fin)),
                   istreambuf_iterator<char>());

        return content;
    }
};

class ScoreWeb {
private:
    vector<KMP> kmpVector;
    double maxScore = 0;
    vector<tuple<string, double>> scores;

public:

    explicit ScoreWeb(const vector<string>& keywords) {
        for(const string& keyword: keywords) {
            kmpVector.emplace_back(KMP(keyword));
        }
    }

    double scoreWebsite(const string& file_path) {
        string content = IO::readFile(file_path);

        double score = 0;
        for(KMP kmp : kmpVector) {
            long match_count = kmp.execute(content);

            // most important part: the score calculation
            score += log(double(match_count + 0.75));
        }

        if(score > maxScore)
            maxScore = score;

        if(score < 0)
            score = 0;

        scores.emplace_back(tuple(file_path, score));

        // sorts the scores from highest to lowest
        sort(scores.begin(), scores.end(), [](const tuple<string, double>& element1, const tuple<string, double>& element2){
            return get<1>(element1) > get<1>(element2);
        });

        return score;
    }

    vector<tuple<string, double>> normalizedScores() {
        vector<tuple<string, double>> normalizedScores;

        for(tuple pair: this->scores) {
            double score = get<1>(pair);

            normalizedScores.emplace_back(tuple<string, double>(get<0>(pair), score/this->maxScore * 100.0));
        }

        return normalizedScores;
    }
};

int main() {
    // find all websites paths
    vector<string> file_paths;

    string dir_path = "../websites";
    if(!exists(dir_path))
        throw invalid_argument("Folder path is wrong. Please specify the folder path again in the code.");

    // Test case insensitive compare
    // cout << KMP::compare('a', 'A') << KMP::compare('A', 'a') << KMP::compare('a', 'a') << KMP::compare('A', 'A') << endl;

    for (const auto & entry : directory_iterator(dir_path)) {
        file_paths.push_back(entry.path().generic_string());
    }

    vector<string> keywords {
        "device", "internet", "thing", "node", "network", "protocol", "data", "comput", "process",
        "distributed", "smart", "automation", "system", "robot", "autonomous", "intelligen"
    };

    ScoreWeb scoreWebInst = ScoreWeb(keywords);

    for(const string& file_path : file_paths) {
        scoreWebInst.scoreWebsite(file_path);
    }

    for(tuple pair: scoreWebInst.normalizedScores()) {
        cout << setw(50) << left << "Filepath: " + get<0>(pair) << left << "Score: " << get<1>(pair) << "% match." << endl;
    }
}