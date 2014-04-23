#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"
#include "db.h"
#include "cmd.h"
#include "bitcase.h"

/*Set command process function to a cmd*/
int cmd_proc(cmd *c, char *cmd_name) {
    /*Command get*/
    if (strcmp(cmd_name, "get")) {
        c->proc = cmd_get_proc;
        return CMD_OK;
    }

    /*Command set*/
    if (strcmp(cmd_name, "set")) {
        c->proc = cmd_set_proc;
        return CMD_OK;
    }

    /*Command delete*/
    if (strcmp(cmd_name, "delete")) {
        c->proc = cmd_delete_proc;
        return CMD_OK;
    }

    /*Command update*/
    if (strcmp(cmd_name, "update")) {
        c->proc = cmd_update_proc;
        return CMD_OK;
    }

    fprintf(stderr, "No such command:%s", cmd_name);
    return CMD_ERROR;
}

/*Parse a command string,return a cmd*/
cmd *cmd_parser(char *cmd_str) {
    int argc, i;
    size_t len;
    cmd *c;
    char *line;
    char *cmd_name, *tmp;

    /*Parse the command name first*/
    line = strstr(cmd_str, "\r\n");
    len = line - cmd_str;
    cmd_name = (char*)malloc((len+1)*sizeof(char));
    strncpy(cmd_name, cmd_str, len);
    cmd_name[len] = '\0';

    /*Then get the arg amount*/
    tmp = line + 2;
    line = strstr(tmp, "\r\n");
    len = line - tmp;
    /*Usually no more than 9*/
    argc = tmp[0] - '0';

    c = (cmd*)malloc(sizeof(cmd));
    c->argc = argc;
    c->argv = (char**)malloc(argc);
    /*Parse each arg*/
    for (i = 0;i < argc;i++) {
        tmp = line + 2;
        line = strstr(tmp, "\r\n");
        len = line - tmp;
        c->argv[i] = (char*)malloc((len+1)*sizeof(char));
        strncpy(c->argv[i], line+2, len);
        c->argv[i][len] = '\0';
    }
    if (cmd_proc(c, cmd_name) == CMD_OK) {
        c->d = server.d;
        free(cmd_name);

        return c;
    }

    return NULL;
}

/*Execute the command*/
int cmd_execute(cmd *c, char *result) {
    /*Just invoke the proc function in the cmd*/
    if (c->proc(c) == CMD_OK) {
        result = (char*)malloc(strlen(c->result)*sizeof(char));
        if (result == NULL)  {
            fprintf(stderr, "Memory alloc error");
            return CMD_ERROR;
        }
        strcpy(result, c->result);
        cmd_free(c);

        return CMD_OK;
    }

    return CMD_ERROR;
}

/*Free a cmd,release it's memory*/
int cmd_free(cmd *c) {
    int i;

    free(c->result);
    for (i = 0;i < c->argc;i++) {
        free(c->argv[i]);
    }
    free(c);

    return CMD_OK;
}

/*Process the get command*/
int cmd_get_proc(cmd *c) {
    bucket *bk;
    char *err_str = "Wrong arg number for command GET,accept only 1";

    /*Command get has 1 arg*/
    if (c->argc != 1) {
        fprintf(stderr, "Command get only accept 1 arg");
        c->result = (void*)malloc(strlen(err_str)*sizeof(char));
        strcpy(c->result, err_str);
        return CMD_ERROR;
    }

    bk = db_get_key(c->d, (void*)c->argv[0]);
    c->result = (void*)malloc(strlen(bk->value)*sizeof(char));
    strcpy(c->result, bk->value);

    return CMD_OK;
}

/*Process the set command*/
int cmd_set_proc(cmd *c) {
    char *ok_str = "OK";
    char *err_str = "Wrong arg number for command SET,accept only 2";

    /*Command set has 2 args*/
    if (c->argc != 2) {
        fprintf(stderr, "Command set only accept 2 args");
        c->result = (void*)malloc(strlen(err_str)*sizeof(char));
        strcpy(c->result, err_str);
        return CMD_ERROR;
    }

    if (db_add_key(c->d, (void*)c->argv[0], (void*)c->argv[1]) == DB_OK) {
        c->result = (void*)malloc(strlen(ok_str)*sizeof(char));
        strcpy(c->result, ok_str);
        return CMD_OK;
    }

    return CMD_ERROR;
}

/*Process the delete command*/
int cmd_delete_proc(cmd *c) {
    char *ok_str = "OK";
    char *err_str = "Wrong arg number for command DELETE,accept only 1";

    /*Command delete has 1 arg*/
    if (c->argc != 1) {
        fprintf(stderr, "Command delete only accept 1 arg");
        c->result = (void*)malloc(strlen(err_str)*sizeof(char));
        strcpy(c->result, err_str);
        return CMD_ERROR;
    }

    if (db_delete_key(c->d, c->argv[0]) == DB_OK) {
        c->result = (void*)malloc(strlen(ok_str)*sizeof(char));
        strcpy(c->result, err_str);
        return CMD_OK;
    }

    return CMD_ERROR;
};

/*Process the udpate command*/
int cmd_update_proc(cmd *c) {
    char *ok_str = "OK";
    char *err_str = "Wrong arg number for command UPDATE,accpet only 2";

    /*Command update has 2 args*/
    if (c->argc != 2) {
        fprintf(stderr, "Command update only accept 2 args");
        c->result = (void*)malloc(strlen(err_str)*sizeof(char));
        strcpy(c->result, err_str);
        return CMD_ERROR;
    }

    if (db_update_key(c->d, c->argv[0], c->argv[1]) == DB_OK) {
        c->result = (void*)malloc(strlen(ok_str)*sizeof(char));
        strcpy(c->result, ok_str);
        return CMD_OK;
    }

    return CMD_ERROR;
}
