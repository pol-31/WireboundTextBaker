/* C++ code produced by gperf version 3.1 */
/* Command-line: gperf -e'$' -t -LC++ -H Hash -N GetPhraseLine -Z PhrasesHash -CEIG -W kWordlist /home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp  */
/* Computed positions: -k'1,7' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 1 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
struct Phrase { char *name; int line; };
#include <string.h>
enum
  {
    TOTAL_KEYWORDS = 52,
    MIN_WORD_LENGTH = 3,
    MAX_WORD_LENGTH = 39,
    MIN_HASH_VALUE = 3,
    MAX_HASH_VALUE = 99
  };

/* maximum key range = 97, duplicates = 0 */

class PhrasesHash
{
private:
  static inline unsigned int Hash (const char *str, size_t len);
public:
  static const struct Phrase *GetPhraseLine (const char *str, size_t len);
};

inline unsigned int
PhrasesHash::Hash (const char *str, size_t len)
{
  static const unsigned char asso_values[] =
    {
     100,100,100,100,100,100,100,100,100,100,
     100,100,100,100,100,100,100,100,100,100,
     100,100,100,100,100,100,100,100,100,100,
     100,100,  0,100,100,100,100,100,100,100,
     100,100,100,100,100,100,100,100,100,100,
     100,100,100,100,100,100,100,100,100,100,
     100,100,100,100,100,  5, 60,  0, 50,100,
     100,  0, 30, 60,100,100, 30, 25, 45, 15,
      25, 15, 10, 20, 10,  5, 30, 60,100, 15,
     100,100,100,100,100,100,100,  0,100,  0,
     100,  5,  0,  0, 25, 35,100,  0,  5,  5,
      25,  0,  5,100,  0,  0,  0, 10,100,100,
     100,  0,100,100,100,100,100,100,100,100,
     100,100,100,100,100,100,100,100,100,100,
     100,100,100,100,100,100,100,100,100,100,
     100,100,100,100,100,100,100,100,100,100,
     100,100,100,100,100,100,100,100,100,100,
     100,100,100,100,100,100,100,100,100,100,
     100,100,100,100,100,100,100,100,100,100,
     100,100,100,100,100,100,100,100,100,100,
     100,100,100,100,100,100,100,100,100,100,
     100,100,100,100,100,100,100,100,100,100,
     100,100,100,100,100,100,100,100,100,100,
     100,100,100,100,100,100,100,100,100,100,
     100,100,100,100,100,100,100,100,100,100,
     100,100,100,100,100,100
    };
  unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[static_cast<unsigned char>(str[6])];
      /*FALLTHROUGH*/
      case 6:
      case 5:
      case 4:
      case 3:
      case 2:
      case 1:
        hval += asso_values[static_cast<unsigned char>(str[0])];
        break;
    }
  return hval;
}

static const struct Phrase kWordlist[] =
  {
    {""}, {""}, {""},
#line 18 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"com",15},
#line 49 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Game",46},
#line 52 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Class",49},
#line 17 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"github",14},
#line 12 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Credits",9},
#line 25 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Godsmack",22},
#line 15 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Copyright",12},
#line 47 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Audio",44},
#line 42 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Accept",39},
#line 44 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"General",41},
#line 46 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Controls",43},
#line 53 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Type",50},
#line 36 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"contract to die",33},
    {""},
#line 10 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Archive",7},
#line 9 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Continue",6},
#line 13 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Quit",10},
#line 43 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Quick",40},
    {""},
#line 11 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Options",8},
    {""},
#line 30 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Testament",27},
#line 50 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Start",47},
    {""},
#line 3 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"aqwertyuiopasdfghjklzxcvbnm",0},
#line 19 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Superman",16},
#line 41 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Prev",38},
#line 16 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"https",13},
    {""},
#line 21 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Pantera",18},
    {""},
#line 26 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Messhugah",23},
#line 48 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Video",45},
#line 34 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"no way punk",31},
#line 14 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Loading",11},
    {""},
#line 8 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Load game",5},
#line 35 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"You take it as a weakness",32},
#line 20 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Lanb of God",17},
#line 23 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Machine Head",20},
    {""},
#line 22 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Hatebreed",19},
#line 33 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Re spect walk are you talking to me",30},
#line 4 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"QWERTYUIOPASDFGHJKLZXCVBNM",1},
#line 38 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"You might guess why am I do this",35},
    {""},
#line 40 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Next",37},
    {""}, {""},
#line 31 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Hew Collosal Hate",28},
#line 39 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Navigate",36},
#line 28 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Dawn",25},
    {""}, {""}, {""},
#line 7 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"New game",4},
#line 29 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Monuments",26},
    {""},
#line 24 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Decapitated",21},
    {""}, {""},
#line 51 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Back",48},
    {""},
#line 54 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Description",51},
#line 27 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Belakor",24},
#line 32 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"London is the capital of Great Britain",29},
#line 45 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Interface",42},
    {""}, {""}, {""}, {""},
#line 5 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"important phrases for prerendering here",2},
    {""}, {""}, {""}, {""},
#line 6 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"Wirebound",3},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""},
#line 37 "/home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp"
    {"I am not going to tell you what happend",34}
  };

const struct Phrase *
PhrasesHash::GetPhraseLine (const char *str, size_t len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      unsigned int key = Hash (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          const char *s = kWordlist[key].name;

          if (*str == *s && !strcmp (str + 1, s + 1))
            return &kWordlist[key];
        }
    }
  return 0;
}
