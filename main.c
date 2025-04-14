#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

// Definição dos tipos de tokens
typedef enum {
    TOKEN_PROGRAMA,
    TOKEN_INICIO,
    TOKEN_FIM,
    TOKEN_RES,
    TOKEN_ID,
    TOKEN_NUM,
    TOKEN_ASSIGN,    // =
    TOKEN_PLUS,      // +
    TOKEN_MINUS,     // -
    TOKEN_MULT,      // *
    TOKEN_DIV,       // /
    TOKEN_LPAREN,    // (
    TOKEN_RPAREN,    // )
    TOKEN_QUOTE,     // "
    TOKEN_COLON,     // :
    TOKEN_EOF,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[64];
} Token;

// Variáveis globais para o lexer
const char *src;   // ponteiro para o código-fonte
int pos = 0;       // posição atual no código-fonte
Token currentToken;

// Função auxiliar para ignorar espaços, tabulações e quebras de linha
void skip_whitespace() {
    while (src[pos] == ' ' || src[pos] == '\t' || src[pos] == '\n') {
        pos++;
    }
}

// Função para identificar palavras reservadas ou identificadores
Token identifier_or_reserved() {
    Token token;
    int start = pos;
    while (isalnum(src[pos])) { // permite letras e dígitos, se desejar, ou só letras
        pos++;
    }
    int len = pos - start;
    strncpy(token.lexeme, src + start, len);
    token.lexeme[len] = '\0';
    
    // Checa palavras reservadas
    if (strcmp(token.lexeme, "PROGRAMA") == 0)
        token.type = TOKEN_PROGRAMA;
    else if (strcmp(token.lexeme, "INICIO") == 0)
        token.type = TOKEN_INICIO;
    else if (strcmp(token.lexeme, "FIM") == 0)
        token.type = TOKEN_FIM;
    else if (strcmp(token.lexeme, "RES") == 0)
        token.type = TOKEN_RES;
    else
        token.type = TOKEN_ID;
    return token;
}

// Função para ler números inteiros
Token number() {
    Token token;
    int start = pos;
    while (isdigit(src[pos])) {
        pos++;
    }
    int len = pos - start;
    strncpy(token.lexeme, src + start, len);
    token.lexeme[len] = '\0';
    token.type = TOKEN_NUM;
    return token;
}

// Função principal do lexer para obter o próximo token
void nextToken() {
    skip_whitespace();
    char c = src[pos];
    
    if (c == '\0') {
        currentToken.type = TOKEN_EOF;
        strcpy(currentToken.lexeme, "EOF");
        return;
    }

    // Identifica letras (palavras reservadas ou identificadores)
    if (isalpha(c)) {
        currentToken = identifier_or_reserved();
        return;
    }
    
    // Identifica números
    if (isdigit(c)) {
        currentToken = number();
        return;
    }
    
    // Símbolos simples
    switch (c) {
        case '=':
            currentToken.type = TOKEN_ASSIGN;
            strcpy(currentToken.lexeme, "=");
            pos++;
            break;
        case '+':
            currentToken.type = TOKEN_PLUS;
            strcpy(currentToken.lexeme, "+");
            pos++;
            break;
        case '-':
            currentToken.type = TOKEN_MINUS;
            strcpy(currentToken.lexeme, "-");
            pos++;
            break;
        case '*':
            currentToken.type = TOKEN_MULT;
            strcpy(currentToken.lexeme, "*");
            pos++;
            break;
        case '/':
            currentToken.type = TOKEN_DIV;
            strcpy(currentToken.lexeme, "/");
            pos++;
            break;
        case '(':
            currentToken.type = TOKEN_LPAREN;
            strcpy(currentToken.lexeme, "(");
            pos++;
            break;
        case ')':
            currentToken.type = TOKEN_RPAREN;
            strcpy(currentToken.lexeme, ")");
            pos++;
            break;
        case '\"':
            currentToken.type = TOKEN_QUOTE;
            strcpy(currentToken.lexeme, "\"");
            pos++;
            break;
        case ':':
            currentToken.type = TOKEN_COLON;
            strcpy(currentToken.lexeme, ":");
            pos++;
            break;
        default:
            currentToken.type = TOKEN_UNKNOWN;
            currentToken.lexeme[0] = c;
            currentToken.lexeme[1] = '\0';
            pos++;
            break;
    }
}

// Função para emitir uma mensagem de erro e encerrar o compilador
void error(const char *msg) {
    fprintf(stderr, "Erro: %s\n", msg);
    exit(EXIT_FAILURE);
}

// Consome o token atual e verifica se ele é do tipo esperado
void consume(TokenType expected) {
    if (currentToken.type == expected) {
        nextToken();
    } else {
        fprintf(stderr, "Erro: Esperava token %d, mas encontrou %d (%s)\n", expected, currentToken.type, currentToken.lexeme);
        exit(EXIT_FAILURE);
    }
}

// Protótipos para as funções de parsing de expressões (respectiva à precedência)
void parse_expr();
void parse_term();
void parse_factor();

// <program> ::= <label> "\n" <start> <statement> "\n" <res_statement> "\n" <end>
void parse_program() {
    // <label> ::= "PROGRAMA " "\"" <var> "\":"
    consume(TOKEN_PROGRAMA);
    // Espera a aspa de abertura:
    consume(TOKEN_QUOTE);
    // O <var> (identificador) – assume que o token atual é um ID
    if (currentToken.type != TOKEN_ID)
        error("Esperava identificador no label do programa.");
    printf("; Definindo o programa: %s\n", currentToken.lexeme);
    consume(TOKEN_ID);
    // Espera a aspa de fechamento
    consume(TOKEN_QUOTE);
    consume(TOKEN_COLON);
    
    // Consome a quebra de linha (opcional – você pode tratar \n como token ou ignorar)
    // Aqui, assumo que o lexer já ignora \n com skip_whitespace()

    // <start> ::= "INICIO"
    consume(TOKEN_INICIO);
    
    // <statement> – pode ser uma ou várias atribuições
    while (currentToken.type == TOKEN_ID) {
        // <ass_statement> ::= <ws> <var> <ws> "=" <ws> <exp>
        // Aqui, o token atual é um identificador (variável)
        char varName[64];
        strcpy(varName, currentToken.lexeme);
        consume(TOKEN_ID);
        consume(TOKEN_ASSIGN);
        printf("; Processando atribuição para %s\n", varName);
        // Para a geração de código, você pode chamar uma função que gera o código para a expressão
        parse_expr();
        // Após calcular a expressão, emita código para armazenar o resultado na variável (ex.: STA varName)
        printf("STA %s\n", varName);
        // Suporta a quebra de linha entre statements
        // nextToken();   // se necessário
    }
    
    // <res_statement> ::= <ws> "RES" <ws> "=" <ws> <exp>
    consume(TOKEN_RES);
    consume(TOKEN_ASSIGN);
    printf("; Processando instrução RES\n");
    parse_expr();
    // Aqui, a geração pode ser, por exemplo, armazenar o resultado em uma área especial
    printf("STA RES\n");  // ou a instrução apropriada

    // <end> ::= "FIM"
    consume(TOKEN_FIM);
}

// <exp> ::= <term> ( <ws> <addop> <ws> <term> )*
void parse_expr() {
    // Começa interpretando o termo
    parse_term();
    // Enquanto o token for um operador de adição ou subtração
    while (currentToken.type == TOKEN_PLUS || currentToken.type == TOKEN_MINUS) {
        TokenType op = currentToken.type;
        consume(op);
        // Para geração de código, pode ser necessário empilhar o valor anterior, ou utilizar registradores.
        // Exemplo de comentário: // push acumulator (dependendo da estratégia)
        parse_term();
        // Emite o código correspondente à operação:
        if (op == TOKEN_PLUS)
            printf("ADD\n");
        else
            printf("SUB\n");
    }
}

// <term> ::= <factor> ( <ws> <mulop> <ws> <factor> )*
void parse_term() {
    parse_factor();
    while (currentToken.type == TOKEN_MULT || currentToken.type == TOKEN_DIV) {
        TokenType op = currentToken.type;
        consume(op);
        parse_factor();
        if (op == TOKEN_MULT)
            printf("MUL\n");
        else
            printf("DIV\n");
    }
}

// <factor> ::= <num> | <var> | "(" <ws> <exp> <ws> ")"
void parse_factor() {
    if (currentToken.type == TOKEN_NUM) {
        // Emite código para carregar o número
        printf("LDC %s\n", currentToken.lexeme);
        consume(TOKEN_NUM);
    } else if (currentToken.type == TOKEN_ID) {
        // Emite código para carregar a variável
        printf("LDA %s\n", currentToken.lexeme);
        consume(TOKEN_ID);
    } else if (currentToken.type == TOKEN_LPAREN) {
        consume(TOKEN_LPAREN);
        parse_expr();
        consume(TOKEN_RPAREN);
    } else {
        error("Erro no factor: token inesperado.");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo-fonte>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    // Carrega o arquivo-fonte (para simplicidade, use um buffer grande)
    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    rewind(fp);
    
    char *buffer = (char *)malloc(fsize + 1);
    fread(buffer, 1, fsize, fp);
    buffer[fsize] = '\0';
    fclose(fp);
    
    // Inicializa o lexer
    src = buffer;
    pos = 0;
    nextToken();
    
    // Inicia o parsing do programa
    parse_program();
    
    // Finaliza gerando o código assembly (os 'printf' acima simulam a emissão do código)
    
    free(buffer);
    return 0;
}
