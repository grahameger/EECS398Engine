#include "String.h"
#include "vector.h"
#include <fstream>
#include <string>

class RankWords {
private:
   Vector<std::string> first_names;
   Vector<std::string> last_names;
   Vector<std::string> nouns;
   Vector<std::pair<std::string, long>> popular_words;
   
   char type;
   long rank = -1;
   
   long getRank(const String &word) {
      //take word. check for existence and type, return rank
      int left = 0;
      int right = (int)popular_words.size();
      
      while ( left <= right )
      {
         int middle = left + ( right - left ) / 2;
         
         if ( popular_words[ middle ].first == word.CString( ) )
         {
            return popular_words[ middle ].second;
         }
         
         if ( popular_words[ middle ].first < word.CString( ) )
         {
            left = middle + 1;
         }
         else
         {
            right = middle - 1;
         }
      }
      return -1;
   }
   
   char getType(const String &word) {
      if(is_noun(word)) {
         return 'n';
      }
      else if(is_first(word)) {
         return 'f';
      }
      else if(is_last(word)) {
         return 'l';
      }
      else {
         return 'x';
      }
   }
   
   bool is_noun(const String &word) {
      return binSearch(nouns, 0, (int)nouns.size(), word);
   }
   
   bool is_first(const String &word) {
      return binSearch(first_names, 0, (int)first_names.size(), word);
   }
   
   bool is_last(const String &word) {
      return binSearch(last_names, 0, (int)last_names.size(), word);
   }
   
   bool binSearch( Vector<std::string> v, int left, int right, String val) {
      while ( left <= right )
      {
         int middle = left + ( right - left ) / 2;
         if ( strncmp(val.CString( ), v[ middle ].c_str(), val.Size()) == 0 )
         {
            return true;
         }
         
         if ( strncmp(v[ middle ].c_str(), val.CString( ), val.Size()) < 0 )
         {
            left = middle + 1;
         }
         else
         {
            right = middle - 1;
         }
      }
      
      return false;
   }

public:
   
   RankWords() {
      std::fstream fs("Popular_nouns.txt"); //existence
      std::string line;
      while (getline(fs, line)) {
         // store each line in the vector
         nouns.push_back(line);
      }
      fs.close();
      
      std::fstream fs2("sorted_first_names.txt"); //existence
      while (getline(fs2, line)) {
         first_names.push_back(line);
      }
      fs.close();
      
      std::fstream fs3("sorted_last_names.txt"); //existence
      while (getline(fs3, line)) {
         // store each line in the vector
         last_names.push_back(line);
      }
      fs.close();
      
      std::fstream fs4("sorted_word_frequency.txt"); //rank, type
      std::string domain;
      long rank;
      while (getline(fs4, line)) {
         // store each line in the vector
         std::size_t pos = line.find(",");
         domain = line.substr(0, pos);
         rank = stoll(line.substr(pos+1));
         //global vector of alexa and rank
         popular_words.push_back(std::pair<std::string, long>(domain,rank));
      }
      fs.close();
   }
   
   std::pair<char, long> getWordRankAndType(const String &word) {
      char type = getType(word);
      long rank = getRank(word);
      return std::pair<char, long>(type, rank);
   }
};