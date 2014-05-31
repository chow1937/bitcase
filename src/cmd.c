#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <uv.h>

#include "hashtable.h"
#include "db.h"
#include "cmd.h"
#include "bitcase.h"
#include "bcmem.h"

/*Init the server commands table*/
int cmd_init_commands(void) {
    server.commands = (hash_table*)bc_malloc(sizeof(hash_table));
    if (ht_alloc(server.commands, CMD_TABLE_SIZE) == HT_OK) {
        /*Add commands into the server.commands hashtable*/
        ht_add(server.commands, "get", cmd_get_proc);
        ht_add(server.commands, "set", cmd_set_proc);
        ht_add(server.commands, "delete", cmd_delete_proc);
        ht_add(server.commands, "update", cmd_update_proc);

        return CMD_OK;
    }

    return CMD_ERROR;
}

/*Set command process function to a cmd*/
int cmd_set_procfun(cmd *c, char *cmd_name) {
    /*Find the process function in server.commands hashtable*/
    bucket *bk = ht_find(server.commands, cmd_name);

    if (bk) {
        if (bk->value) {
            c->proc = bk->value;
            return CMD_OK;
        }
    }

    fprintf(stderr, "No such command:%s\n", cmd_name);
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
    cmd_name = (char*)bc_malloc((len+1)*sizeof(char));
    strncpy(cmd_name, cmd_str, len);
    cmd_name[len] = '\0';

    /*Then get the arg amount*/
    tmp = line + 2;
    line = strstr(tmp, "\r\n");
    /*Usually no more than 9*/
    argc = tmp[0] - '0';

    c = (cmd*)bc_malloc(sizeof(cmd));
    c->argc = argc;
    c->argv = (char**)bc_malloc(argc);
    /*Parse each arg*/
    for (i = 0;i < argc;i++) {
        tmp = line + 2;
        line = strstr(tmp, "\r\n");
        len = line - tmp;
        c->argv[i] = (char*)bc_malloc((len+1)*sizeof(char));
        strncpy(c->argv[i], tmp, len);
        c->argv[i][len] = '\0';
    }
    if (cmd_set_procfun(c, cmd_name) == CMD_OK) {
        c->d = server.d;
        bc_free(cmd_name);

        return c;
    }

    return NULL;
}

/*Execute the command*/
int cmd_execute(cmd *c) {
    /*Just invoke the proc function in the cmd*/
    return c->proc(c);
}

/*Free a cmd,release it's memory*/
int cmd_free(cmd *c) {
    int i;

    bc_free(c->result);
    for (i = 0;i < c->argc;i++) {
        bc_free(c->argv[i]);
    }
    bc_free(c);

    return CMD_OK;
}

/*Process the get command*/
int cmd_get_proc(cmd *c) {
    bucket *bk;
    char *err_str = "Wrong arg number for command GET,accept only 1";
    char *fail_str = "Key not exists";

    /*Command get has 1 arg*/
    if (c->argc != 1) {
        fprintf(stderr, "Command get only accept 1 arg,got %d\n", c->argc);
        c->result = (void*)bc_malloc(strlen(err_str)*sizeof(char));
        strcpy(c->result, err_str);
        return CMD_ERROR;
    }

    bk = db_get_key(c->d, (void*)c->argv[0]);
    if (bk) {
        c->result = (void*)bc_malloc(strlen(bk->value)*sizeof(char));
        strcpy(c->result, bk->value);
    } else {
        /*Get key fail,return fail info*/
        c->result = (void*)bc_malloc(strlen(fail_str));
        strcpy(c->result, fail_str);
        return CMD_ERROR;
    }

    return CMD_OK;
}

/*Process the set command*/
int cmd_set_proc(cmd *c) {
    char *ok_str = "OK";
    char *err_str = "Wrong arg number for command SET,accept only 2";
    char *fail_str = "key exists,add fail";

    /*Command set has 2 args*/
    if (c->argc != 2) {
        fprintf(stderr, "Command set only accept 2 args\n");
        c->result = (void*)bc_malloc(strlen(err_str)*sizeof(char));
        strcpy(c->result, err_str);
        return CMD_ERROR;
    }

    if (db_add_key(c->d, (void*)c->argv[0], (void*)c->argv[1]) == DB_OK) {
        /*Add key success, return success info*/
        c->result = (void*)bc_malloc(strlen(ok_str)*sizeof(char));
        strcpy(c->result, ok_str);
        return CMD_OK;
    } else {
        /*Key exists, return fail info*/
        c->result = (void*)bc_malloc(strlen(fail_str)*sizeof(char));
        strcpy(c->result, fail_str);
    }

    return CMD_ERROR;
}

/*Process the delete command*/
int cmd_delete_proc(cmd *c) {
    char *ok_str = "OK";
    char *err_str = "Wrong arg number for command DELETE,accept only 1";
    char *fail_str = "Key not exists";

    /*Command delete has 1 arg*/
    if (c->argc != 1) {
        fprintf(stderr, "Command delete only accept 1 arg\n");
        c->result = (void*)bc_malloc(strlen(err_str)*sizeof(char));
        strcpy(c->result, err_str);
        return CMD_ERROR;
    }

    if (db_delete_key(c->d, c->argv[0]) == DB_OK) {
        c->result = (void*)bc_malloc(strlen(ok_str)*sizeof(char));
        strcpy(c->result, ok_str);
        return CMD_OK;
    } else {
        /*Delete key error,not exists,return fail info*/
        c->result = (void*)bc_malloc(strlen(fail_str)*sizeof(char));
        strcpy(c->result, fail_str);
    }

    return CMD_ERROR;
}

/*Process the udpate command*/
int cmd_update_proc(cmd *c) {
    char *ok_str = "OK";
    char *err_str = "Wrong arg number for command UPDATE,accpet only 2";
    char *fail_str = "Key not exists";

    /*Command update has 2 args*/
    if (c->argc != 2) {
        fprintf(stderr, "Command update only accept 2 args\n");
        c->result = (void*)bc_malloc(strlen(err_str)*sizeof(char));
        strcpy(c->result, err_str);
        return CMD_ERROR;
    }

    if (db_update_key(c->d, c->argv[0], c->argv[1]) == DB_OK) {
        c->result = (void*)bc_malloc(strlen(ok_str)*sizeof(char));
        strcpy(c->result, ok_str);
        return CMD_OK;
    } else {
        /*Update error,key not exists,return fail info*/
        c->result = (void*)bc_malloc(strlen(fail_str)*sizeof(char));
        strcpy(c->result, fail_str);
    }

    return CMD_ERROR;
}
