#include "parse.h"

PrepareResult prepare_insert(InputBuffer* inputBuffer, Statement* statement){
    statement->type = STATEMENT_INSERT;
    char* keyword = strtok(inputBuffer->buffer, " ");
    char* id_string = strtok(NULL, " ");
    char* username = strtok(NULL, " ");
    char* email = strtok(NULL, " ");

    if (id_string == NULL || username == NULL || email == NULL){
        return PREPARE_SYNTAX_ERROR;
    }

    int id = atoi(id_string);
    if (id < 0 ){
        return PREPARE_NEGATIVE_ID;
    }
    if (strlen(username) > COLUMN_USERNAME_SIZE){
        return PREPARE_STRING_TOO_LONG;
    }
    if (strlen(email) > COLUMN_EMAIL_SIZE){
        return PREPARE_STRING_TOO_LONG;
    }

    statement-> row_to_insert.id = id;
    strcpy(statement->row_to_insert.username, username);
    strcpy(statement->row_to_insert.email, email);
    return PREPARE_SUCCESS;
}

PrepareResult prepare_statement(InputBuffer* inputBuffer, Statement* statement){
    if(strncmp(inputBuffer->buffer, "insert", strlen("insert")) == 0){
        return prepare_insert(inputBuffer, statement);
    }
    if(strncmp(inputBuffer->buffer, "select", strlen("select")) == 0){
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    return PREPARE_UNRECOGNIZED_STATEMENT;
}