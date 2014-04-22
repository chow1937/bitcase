#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"
#include "db.h"
#include "cmd.h"

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
    c->result = bk->value;

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
}
