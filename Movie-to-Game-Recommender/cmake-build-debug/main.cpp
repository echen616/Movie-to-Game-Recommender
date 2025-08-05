#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <queue>
#include <algorithm>
#include <cctype>
using namespace std;

const int T = 3; // Minimum degree (defines range for number of keys)

class BTreeNode {
public:
    vector<string> keys;
    vector<vector<string>> titles; // each key corresponds to a genre, so this stores a list of titles per genre
    vector<BTreeNode*> children;
    bool leaf;

    BTreeNode(bool isLeaf) : leaf(isLeaf) {}

    void traverse() {
        for (int i = 0; i < keys.size(); i++) {
            if (!leaf) children[i]->traverse();
            cout << keys[i] << ": ";
            for (const string& title : titles[i]) cout << title << ", ";
            cout << endl;
        }
        if (!leaf) children[keys.size()]->traverse();
    }

    vector<string> search(const string& genre) {
        int i = 0;
        while (i < keys.size() && genre > keys[i]) i++;
        if (i < keys.size() && keys[i] == genre) return titles[i];
        if (leaf) return {};
        return children[i]->search(genre);
    }

    void insertNonFull(const string& genre, const string& title);
    void splitChild(int i, BTreeNode* y);
};

class BTree {
    BTreeNode* root;
public:
    BTree() { root = new BTreeNode(true); }

    void insert(const string& genre, const string& title);

    vector<string> search(const string& genre) {
        return root->search(genre);
    }
};

void BTreeNode::insertNonFull(const string& genre, const string& title) {
    int i = keys.size() - 1;
    if (leaf) {
        while (i >= 0 && genre < keys[i]) i--;
        if (i >= 0 && keys[i] == genre) {
            titles[i].push_back(title);
            return;
        }
        keys.insert(keys.begin() + i + 1, genre);
        titles.insert(titles.begin() + i + 1, {title});
    } else {
        while (i >= 0 && genre < keys[i]) i--;
        i++;
        if (children[i]->keys.size() == 2 * T - 1) {
            splitChild(i, children[i]);
            if (genre > keys[i]) i++;
        }
        children[i]->insertNonFull(genre, title);
    }
}

void BTreeNode::splitChild(int i, BTreeNode* y) {
    BTreeNode* z = new BTreeNode(y->leaf);

    string middleKey = y->keys[T - 1];
    vector<string> middleTitles = y->titles[T - 1];

    z->keys.assign(y->keys.begin() + T, y->keys.end());
    z->titles.assign(y->titles.begin() + T, y->titles.end());

    y->keys.resize(T - 1);
    y->titles.resize(T - 1);

    if (!y->leaf) {
        z->children.assign(y->children.begin() + T, y->children.end());
        y->children.resize(T);
    }

    this->children.insert(this->children.begin() + i + 1, z);
    this->keys.insert(this->keys.begin() + i, middleKey);
    this->titles.insert(this->titles.begin() + i, middleTitles);
}

void BTree::insert(const string& genre, const string& title) {
    if (root->keys.size() == 2 * T - 1) {
        BTreeNode* s = new BTreeNode(false);
        s->children.push_back(root);
        s->splitChild(0, root);
        root = s;
    }
    root->insertNonFull(genre, title);
}

class Graph {
public:
    void addEdge(const string& genre, const string& title) {
        adj[genre].push_back(title);
    }

    vector<string> recommend(const string& genre) {
        vector<string> result;
        if (adj.find(genre) != adj.end()) {
            for (const auto& title : adj[genre]) {
                result.push_back(title);
            }
        }
        return result;
    }

private:
    map<string, vector<string>> adj;
};

// Improved CSV parsing function
vector<string> parseCSVLine(const string& line) {
    vector<string> result;
    string current = "";
    bool inQuotes = false;

    for (size_t i = 0; i < line.length(); i++) {
        char c = line[i];

        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            result.push_back(current);
            current = "";
        } else {
            current += c;
        }
    }
    result.push_back(current); // Add the last field
    return result;
}

vector<string> split(const string& line, char delimiter) {
    vector<string> tokens;
    stringstream ss(line);
    string temp;
    while (getline(ss, temp, delimiter)) {
        tokens.push_back(temp);
    }
    return tokens;
}

string toLower(const string& s) {
    string out;
    for (char c : s) out += tolower(c);
    return out;
}

// Trim whitespace from string
string trim(const string& str) {
    size_t first = str.find_first_not_of(' ');
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

string mapSimilarGenre(const string& g) {
    string genre = toLower(g);
    if (genre.find("war") != string::npos) return "shooter";
    if (genre.find("crime") != string::npos) return "adventure";
    if (genre.find("mystery") != string::npos) return "puzzle";
    if (genre.find("sci") != string::npos) return "sci-fi";
    if (genre.find("romance") != string::npos) return "drama";
    if (genre.find("drama") != string::npos) return "story";
    if (genre.find("fantasy") != string::npos) return "rpg";
    if (genre.find("thriller") != string::npos) return "horror";
    if (genre.find("animation") != string::npos || genre.find("cartoon") != string::npos) return "platformer";
    if (genre.find("comedy") != string::npos) return "casual";
    if (genre.find("biography") != string::npos || genre.find("history") != string::npos) return "educational";
    if (genre.find("sport") != string::npos || genre.find("sports") != string::npos) return "sports";
    if (genre.find("music") != string::npos || genre.find("musical") != string::npos) return "rhythm";
    if (genre.find("documentary") != string::npos) return "simulation";
    return genre;
}

struct MovieData {
    vector<string> originalGenres;
    vector<string> mappedGenres;
};

map<string, MovieData> loadMovieGenres(const string& filename) {
    map<string, MovieData> movieData;
    ifstream file(filename);

    if (!file.is_open()) {
        cout << "Error: Could not open file " << filename << endl;
        return movieData;
    }

    string line;
    getline(file, line); // skip header

    while (getline(file, line)) {

        // Use improved CSV parsing
        vector<string> fields = parseCSVLine(line);

        if (fields.size() < 2) {
            cout << "Skipping malformed line: " << line << endl;
            continue;
        }

        string title = trim(fields[0]);
        string genreString = trim(fields[1]);

        // Remove quotes if present
        if (title.front() == '"' && title.back() == '"') {
            title = title.substr(1, title.length() - 2);
        }
        if (genreString.front() == '"' && genreString.back() == '"') {
            genreString = genreString.substr(1, genreString.length() - 2);
        }

        vector<string> originalGenres = split(genreString, '|');
        vector<string> mappedGenres;

        for (auto& g : originalGenres) {
            g = trim(g);
            mappedGenres.push_back(mapSimilarGenre(toLower(g)));
        }

        string lowerTitle = toLower(title);
        movieData[lowerTitle] = {originalGenres, mappedGenres};
    }
    return movieData;
}

vector<pair<string, string>> loadGameList(const string& filename) {
    vector<pair<string, string>> games;
    ifstream file(filename);

    if (!file.is_open()) {
        cout << "Error: Could not open file " << filename << endl;
        return games;
    }

    string line;
    getline(file, line); // skip header

    while (getline(file, line)) {
        size_t comma = line.find(',');
        if (comma == string::npos) continue;

        string gameTitle = line.substr(0, comma);
        string genre = line.substr(comma + 1);

        // Remove quotes
        gameTitle.erase(remove(gameTitle.begin(), gameTitle.end(), '"'), gameTitle.end());
        genre.erase(remove(genre.begin(), genre.end(), '"'), genre.end());

        gameTitle = trim(gameTitle);
        genre = trim(genre);
        genre = mapSimilarGenre(toLower(genre));

        games.push_back({genre, gameTitle});
    }
    return games;
}

int main() {
    cout << "Loading movies..." << endl;
    map<string, MovieData> movies = loadMovieGenres("movies_100k.csv");

    cout << "Loading games..." << endl;
    vector<pair<string, string>> games = loadGameList("gameslist.csv");

    BTree gameTree;
    Graph gameGraph;

    cout << "Building game database..." << endl;
    for (const auto& [genre, title] : games) {
        gameTree.insert(genre, title);
        gameGraph.addEdge(genre, title);
    }

    while (true) {
        cout << "\nEnter a movie title (or type 'exit' to quit): ";
        string input;
        getline(cin, input);

        if (input == "exit") {
            cout << "Goodbye!\n";
            break;
        }

        string lowerInput = toLower(trim(input));
        cout << "Searching for: '" << lowerInput << "'" << endl;

        if (movies.find(lowerInput) == movies.end()) {
            cout << "Movie not found. Available movies (first 5):" << endl;
            int count = 0;
            for (const auto& [title, data] : movies) {
                if (count++ >= 5) break;
                cout << "- " << title << endl;
            }
            continue;
        }

        MovieData movieData = movies[lowerInput];
        cout << "\nMovie: " << input << endl;
        cout << "Genres: ";
        for (const auto& g : movieData.originalGenres) cout << "[" << g << "] ";
        cout << "\n";

        cout << "Choose recommendation method: (1) B-Tree or (2) Graph: ";
        int choice;
        cin >> choice;
        cin.ignore();

        if (choice == 1) {
            cout << "\n=== Game Recommendations (B-Tree) ===\n";
            for (const auto& mappedGenre : movieData.mappedGenres) {
                cout << "\nGames for genre '" << mappedGenre << "':" << endl;
                vector<string> recs = gameTree.search(mappedGenre);
                if (recs.empty()) {
                    cout << "  No games found for this genre." << endl;
                } else {
                    for (int i = 0; i < min(3, (int)recs.size()); i++) {
                        cout << "  - " << recs[i] << "\n";
                    }
                }
            }
        } else if (choice == 2) {
            cout << "\n=== Game Recommendations (Graph) ===\n";
            for (const auto& mappedGenre : movieData.mappedGenres) {
                cout << "\nGames for genre '" << mappedGenre << "':" << endl;
                vector<string> recs = gameGraph.recommend(mappedGenre);
                if (recs.empty()) {
                    cout << "  No games found for this genre." << endl;
                } else {
                    for (int i = 0; i < min(3, (int)recs.size()); i++) {
                        cout << "  - " << recs[i] << "\n";
                    }
                }
            }
        } else {
            cout << "Invalid choice.\n";
        }
    }

    return 0;
}