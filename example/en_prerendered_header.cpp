/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf /home/pavlo/Desktop/prerender/en_prerendered_header_temp.cpp  */
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


#define TOTAL_KEYWORDS 52
#define MIN_WORD_LENGTH 3
#define MAX_WORD_LENGTH 39
#define MIN_HASH_VALUE 3
#define MAX_HASH_VALUE 99
/* maximum key range = 97, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash (register const char *str, register size_t len)
{
  static unsigned char asso_values[] =
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
  register unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[6]];
      /*FALLTHROUGH*/
      case 6:
      case 5:
      case 4:
      case 3:
      case 2:
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval;
}

const char *
in_word_set (register const char *str, register size_t len)
{
  static const char * wordlist[] =
    {
      "", "", "",
      "com",
      "Game",
      "Class",
      "github",
      "Credits",
      "Godsmack",
      "Copyright",
      "Audio",
      "Accept",
      "General",
      "Controls",
      "Type",
      "contract to die",
      "",
      "Archive",
      "Continue",
      "Quit",
      "Quick",
      "",
      "Options",
      "",
      "Testament",
      "Start",
      "",
      "aqwertyuiopasdfghjklzxcvbnm",
      "Superman",
      "Prev",
      "https",
      "",
      "Pantera",
      "",
      "Messhugah",
      "Video",
      "no way punk",
      "Loading",
      "",
      "Load game",
      "You take it as a weakness",
      "Lanb of God",
      "Machine Head",
      "",
      "Hatebreed",
      "Re spect walk are you talking to me",
      "QWERTYUIOPASDFGHJKLZXCVBNM",
      "You might guess why am I do this",
      "",
      "Next",
      "", "",
      "Hew Collosal Hate",
      "Navigate",
      "Dawn",
      "", "", "",
      "New game",
      "Monuments",
      "",
      "Decapitated",
      "", "",
      "Back",
      "",
      "Description",
      "Belakor",
      "London is the capital of Great Britain",
      "Interface",
      "", "", "", "",
      "important phrases for prerendering here",
      "", "", "", "",
      "Wirebound",
      "", "", "", "", "", "", "", "", "",
      "", "", "", "", "", "", "", "", "",
      "",
      "I am not going to tell you what happend"
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register unsigned int key = hash (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          register const char *s = wordlist[key];

          if (*str == *s && !strcmp (str + 1, s + 1))
            return s;
        }
    }
  return 0;
}
