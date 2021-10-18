/* Scanner
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "reader.h"
#include "charcode.h"
#include "token.h"
#include "error.h"


extern int lineNo;
extern int colNo;
extern int currentChar;

extern CharCode charCodes[];

/***************************************************************/

void skipBlank() {
  while(currentChar != EOF && charCodes[currentChar] == CHAR_SPACE){
    readChar();
  }
}

void skipComment() {
  do{
    do{
    readChar();
    }while(currentChar != EOF && charCodes[currentChar] != CHAR_TIMES);
    if(currentChar == EOF){
      error(ERR_ENDOFCOMMENT, lineNo, colNo);
      return;
    }
    readChar();
  }while(currentChar != EOF && charCodes[currentChar] != CHAR_SLASH);
  if(currentChar == EOF){
    error(ERR_ENDOFCOMMENT, lineNo, colNo);
  }else{
    readChar();
  }
}

Token* readIdentKeyword(void) {
  Token *token = makeToken(TK_IDENT, lineNo, colNo);
  int count = 0;
  while(currentChar != EOF && (charCodes[currentChar] == CHAR_LETTER || charCodes[currentChar] == CHAR_DIGIT)) {
      token->string[count] = currentChar;
      count++;
      readChar();
  }
  if(count >= 15){
    token->string[15] = '\0';
  } else {
    token->string[count] = '\0';
  }
  TokenType kw = checkKeyword(token->string);
  int i, check;
  check = 0;
  char *s = token->string;
  for (i = 0; s[i]!='\0'; i++) {
    if(s[i] >= 'a' && s[i] <= 'z') {
      check = 1;
      break;
    }
  }
  if (kw != TK_NONE && check == 0) {
    token->tokenType = kw;
  }
  return token;
}

Token* readNumber(void) {
  Token *token = makeToken(TK_NUMBER, lineNo, colNo);
  // int ln = lineNo;
  // int cn = colNo;
  int count = 0;
  while(currentChar != EOF && charCodes[currentChar] == CHAR_DIGIT){
    token->string[count] = currentChar;
    count++;
    readChar();
  }
  token->string[count] = '\0';
  // error here
  if(strlen(token->string) == 10){
    char buffer[11];
    sprintf(buffer, "%d", INT_MAX);
    if(strcmp(token->string,buffer) <= 0){
      token->value = atoi(token->string);
    }else{
      // used to generate error
      // error(ERR_NUMBERTOOLAGRE, ln, cn);
    }
  }
  if(strlen(token->string) < 10){
    token->value = atoi(token->string);
  }
  if(strlen(token->string) > 10){
    // used to generate error
    // error(ERR_NUMBERTOOLAGRE, ln, cn);
  }
  return token;
}

Token* readConstChar(void) {
  Token *token;
  int ln = lineNo;
  int cn = colNo;
  int count = 0;
  int validBackSlash = 0;
  token = makeToken(TK_CHAR, ln, cn);
  readChar();
  if(currentChar != 92) validBackSlash = 1; // for case '@\'
  if(charCodes[currentChar] == CHAR_SINGLEQUOTE){
    error(ERR_INVALIDCHARCONSTANT, ln, cn);
    //return makeToken();          // ??
  }
  if(currentChar != EOF){
    while(charCodes[currentChar] != CHAR_SINGLEQUOTE || count == 0){  
      if(validBackSlash != 1){ // for case '\?' if '\\\' will be wrong
        if(currentChar == 92){ // '\'
          validBackSlash = 1;
          readChar();          // '\?'
          if(currentChar == EOF){
            error(ERR_INVALIDCHARCONSTANT, ln, cn);
            return makeToken(TK_CHAR, ln, cn);
          }
          if(currentChar == 92 || charCodes[currentChar] == CHAR_SINGLEQUOTE){
            // for case '\\' or '\''
            continue;
          }else{
            // for case '\@'
            token->string[0] = 92;
            count++;
          }
        }
      }
      token->string[count] = currentChar;
      count++;
      readChar();
      if(currentChar == EOF){
        error(ERR_INVALIDCHARCONSTANT, ln, cn);
        return makeToken(TK_CHAR, ln, cn);
      }
    }
    token->string[count] = '\0';
    if(count != 1){
      error(ERR_INVALIDCHARCONSTANT, ln, cn);
    }
    readChar();
    return token;
  } else {
    error(ERR_INVALIDCHARCONSTANT, ln, cn);
    return makeToken(TK_CHAR, ln, cn);
  }
}

Token* getToken(void) {
  Token *token;
  int ln, cn;

  if (currentChar == EOF) 
    return makeToken(TK_EOF, lineNo, colNo);
  
  switch (charCodes[currentChar]) {
  case CHAR_SPACE: skipBlank(); return getToken();
  case CHAR_LETTER: return readIdentKeyword();
  case CHAR_DIGIT: return readNumber();
  case CHAR_PLUS: 
    token = makeToken(SB_PLUS, lineNo, colNo);
    readChar(); 
    return token;
  case CHAR_MINUS:
    token = makeToken(SB_MINUS, lineNo, colNo);
    readChar();
    return token;
  case CHAR_TIMES:
    token = makeToken(SB_TIMES, lineNo, colNo);
    readChar();
    return token;
  case CHAR_SLASH:
    ln = lineNo;
    cn = colNo;
    readChar();
    if ((currentChar != EOF) && (charCodes[currentChar] == CHAR_TIMES)) {
      skipComment();
      return getToken();
    } else return makeToken(SB_SLASH, ln, cn);
  case CHAR_GT:
    ln = lineNo;
    cn = colNo;
    readChar();
    if ((currentChar != EOF) && (charCodes[currentChar] == CHAR_EQ)) {
      readChar();
      return makeToken(SB_GE, ln, cn);
    } else return makeToken(SB_GT, ln, cn);
  case CHAR_LT:
    ln = lineNo;
    cn = colNo;
    readChar();
    if ((currentChar != EOF) && (charCodes[currentChar] == CHAR_EQ)) {
      readChar();
      return makeToken(SB_LE, ln, cn);
    } else return makeToken(SB_LT, ln, cn);
  case CHAR_EXCLAIMATION:
    ln = lineNo;
    cn = colNo;
    readChar();
    if (currentChar != EOF) {
      if(charCodes[currentChar] == CHAR_EQ){
        readChar();
        return makeToken(SB_NEQ, ln, cn);
      } else {
        readChar();
        error(ERR_INVALIDSYMBOL, ln, cn);
        return makeToken(TK_NONE, ln, cn);
      }
    } else {
      error(ERR_INVALIDSYMBOL, ln, cn);
      return makeToken(TK_NONE, ln, cn);
    }
  case CHAR_RPAR:
    token = makeToken(SB_RPAR, lineNo, colNo);
    readChar();
    return token;
  case CHAR_EQ:
    ln = lineNo;
    cn = colNo;
    readChar();
    if ((currentChar != EOF) && (charCodes[currentChar] == CHAR_EQ)) {
      readChar();
      return makeToken(SB_EQ, ln, cn);
    } else return makeToken(SB_ASSIGN, ln, cn);
  case CHAR_COMMA:
    token = makeToken(SB_COMMA, lineNo, colNo);
    readChar();
    return token;
  case CHAR_SEMICOLON:
    token = makeToken(SB_SEMICOLON, lineNo, colNo);
    readChar();
    return token;
  case CHAR_PERIOD:
    ln = lineNo;
    cn = colNo;
    readChar();
    if ((currentChar != EOF) && (charCodes[currentChar] == CHAR_RPAR)) {
      readChar();
      return makeToken(SB_RSEL, ln, cn);
    } else return makeToken(SB_PERIOD, ln, cn);
  case CHAR_COLON:
    token = makeToken(SB_COLON, lineNo, colNo);
    readChar();
    return token;
  case CHAR_SINGLEQUOTE: return readConstChar();
  case CHAR_LPAR: 
    ln = lineNo;
    cn = colNo;
    readChar();
    if(currentChar != EOF && charCodes[currentChar] == CHAR_PERIOD){
        readChar();
        return makeToken(SB_LSEL, ln, cn);
    } else return makeToken(SB_LPAR, ln, cn);
  default:
    token = makeToken(TK_NONE, lineNo, colNo);
    error(ERR_INVALIDSYMBOL, lineNo, colNo);
    readChar(); 
    return token;
  }
}


/******************************************************************/

void printToken(Token *token) {

  printf("%d-%d:", token->lineNo, token->colNo);

  switch (token->tokenType) {
  case TK_NONE: printf("TK_NONE\n"); break;
  case TK_IDENT: printf("TK_IDENT(%s)\n", token->string); break;
  case TK_NUMBER: printf("TK_NUMBER(%s)\n", token->string); break;
  case TK_CHAR: printf("TK_CHAR(\'%s\')\n", token->string); break;
  case TK_EOF: printf("TK_EOF\n"); break;

  case KW_PROGRAM: printf("KW_PROGRAM\n"); break;
  case KW_CONST: printf("KW_CONST\n"); break;
  case KW_TYPE: printf("KW_TYPE\n"); break;
  case KW_VAR: printf("KW_VAR\n"); break;
  case KW_INTEGER: printf("KW_INTEGER\n"); break;
  case KW_CHAR: printf("KW_CHAR\n"); break;
  case KW_ARRAY: printf("KW_ARRAY\n"); break;
  case KW_OF: printf("KW_OF\n"); break;
  case KW_FUNCTION: printf("KW_FUNCTION\n"); break;
  case KW_PROCEDURE: printf("KW_PROCEDURE\n"); break;
  case KW_BEGIN: printf("KW_BEGIN\n"); break;
  case KW_END: printf("KW_END\n"); break;
  case KW_CALL: printf("KW_CALL\n"); break;
  case KW_IF: printf("KW_IF\n"); break;
  case KW_THEN: printf("KW_THEN\n"); break;
  case KW_ELSE: printf("KW_ELSE\n"); break;
  case KW_WHILE: printf("KW_WHILE\n"); break;
  case KW_DO: printf("KW_DO\n"); break;
  case KW_FOR: printf("KW_FOR\n"); break;
  case KW_TO: printf("KW_TO\n"); break;

  case SB_SEMICOLON: printf("SB_SEMICOLON\n"); break;
  case SB_COLON: printf("SB_COLON\n"); break;
  case SB_PERIOD: printf("SB_PERIOD\n"); break;
  case SB_COMMA: printf("SB_COMMA\n"); break;
  case SB_ASSIGN: printf("SB_ASSIGN\n"); break;
  case SB_EQ: printf("SB_EQ\n"); break;
  case SB_NEQ: printf("SB_NEQ\n"); break;
  case SB_LT: printf("SB_LT\n"); break;
  case SB_LE: printf("SB_LE\n"); break;
  case SB_GT: printf("SB_GT\n"); break;
  case SB_GE: printf("SB_GE\n"); break;
  case SB_PLUS: printf("SB_PLUS\n"); break;
  case SB_MINUS: printf("SB_MINUS\n"); break;
  case SB_TIMES: printf("SB_TIMES\n"); break;
  case SB_SLASH: printf("SB_SLASH\n"); break;
  case SB_LPAR: printf("SB_LPAR\n"); break;
  case SB_RPAR: printf("SB_RPAR\n"); break;
  case SB_LSEL: printf("SB_LSEL\n"); break;
  case SB_RSEL: printf("SB_RSEL\n"); break;
  }
}

int scan(char *fileName) {
  Token *token;

  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;

  token = getToken();
  while (token->tokenType != TK_EOF) {
    printToken(token);
    free(token);
    token = getToken();
  }

  free(token);
  closeInputStream();
  return IO_SUCCESS;
}

/******************************************************************/

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    printf("scanner: no input file.\n");
    return -1;
  }

  if (scan(argv[1]) == IO_ERROR) {
    printf("Can\'t read input file!\n");
    return -1;
  }
    
  return 0;
}
