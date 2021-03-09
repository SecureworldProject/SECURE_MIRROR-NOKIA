#include "context.h"
/*
void printContext(struct Context ctx) {
    // Folders
    for (int i = 0; i < _msize(ctx.folders)/sizeof(struct Folder); i++) {
        printf("Folder\n");
        printf("  Path: %s\n", ctx.folders[i]->path);
        printf("  Mount point: %s\n", ctx.folders[i]->mount_point);
        printf("  Driver: %d\n", ctx.folders[i]->driver);
        printf("  Protection\n");
        printf("    Op table: %s\n", ctx.folders[i]->protection->op_table);
        /*
        printf("\nINICIO-INICIO-INICIO-INICIO-INICIO-INICIO-INICIO-INICIO-INICIO-INICIO\n");
        printf("num_groups = %d \n", num_groups);
        printf("sizeof(*challenge groups) = %llu \n", sizeof(*(context.folders[index]->protection->challenge_groups)));
        printf("sizeof(challenge groups) = %llu \n", sizeof(context.folders[index]->protection->challenge_groups));
        printf("sizeof(&challenge groups) = %llu \n", sizeof(&(context.folders[index]->protection->challenge_groups)));
        printf("_msize(challenge groups) = %llu \n", _msize(context.folders[index]->protection->challenge_groups));
        printf("sizeof(char*) = %llu \n", sizeof(char*));
        printf("num_groups * sizeof(char*) = %llu \n", num_groups * sizeof(char*));
        printf("FIN-FIN-FIN-FIN-FIN-FIN-FIN-FIN-FIN-FIN-FIN-FIN-FIN-FIN-FIN-FIN-FIN-FIN\n\n");
        */
        /*
        printf("sizeof(ctx.folders[i]->protection->challenge_groups) = %llu \n", sizeof(ctx.folders[i]->protection->challenge_groups));
        printf("sizeof(ctx.folders[i]->protection->challenge_groups[0]) = %llu \n", sizeof(ctx.folders[i]->protection->challenge_groups)[0]);
        printf("sizeof(*(ctx.folders[i]->protection->challenge_groups)) = %llu \n", sizeof(*(ctx.folders[i]->protection->challenge_groups)));
        printf("sizeof(ctx.folders[i]->protection->challenge_groups)/sizeof(char*) = %llu \n", sizeof(ctx.folders[i]->protection->challenge_groups) / sizeof(char*));
        printf("XXX = %d\n", sizeof(ctx.folders[i]->protection->challenge_groups) / sizeof(char*));
        
        printf("    Challenge groups: ");
        for (int j = 0; j < sizeof(ctx.folders[i]->protection->challenge_groups) / sizeof(char*); j++) {
            //printf("%s%s", ctx.folders[i]->protection->challenge_groups[j], (j + 1 < sizeof(ctx.folders[i]->protection->challenge_groups)) ? ", " : "\n");
            printf("%s\n", ctx.folders[i]->protection->challenge_groups[j]);
        }
        printf("    aaa:\n");*/
/*
        printf("    Challenge groups: ");
        for (int j = 0; j < _msize(ctx.folders[i]->protection->challenge_groups) / sizeof(char*); j++) {
            printf("%s%s", ctx.folders[i]->protection->challenge_groups[j], (j + 1 < _msize(ctx.folders[i]->protection->challenge_groups)/sizeof(char*)) ? ", " : "\n");
        }

        printf("    Cipher: %c\n", *(ctx.folders[i]->protection->cipher));
    }

    // Folders

}*/