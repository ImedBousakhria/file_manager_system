#include <stdio.h>
#include <stdlib.h>

void run_cmd(const char* cmd) {
    printf("\n🔸 Running: %s\n", cmd);
    int result = system(cmd);
    if (result != 0) {
        printf("❌ Command failed: %s\n", cmd);
    }
}

int main() {
    const char* exec = "main.exe";

    printf("🧪 Starting File System Test Sequence...\n");

    char cmd[512];

    // --- List ---
    snprintf(cmd, sizeof(cmd), "%s -l", exec); run_cmd(cmd);

    // --- Directory Creation ---
    snprintf(cmd, sizeof(cmd), "%s -d home / 0 755", exec); run_cmd(cmd);
    snprintf(cmd, sizeof(cmd), "%s -d etc / 0 755", exec); run_cmd(cmd);
    snprintf(cmd, sizeof(cmd), "%s -d user1 /home 0 755", exec); run_cmd(cmd);
    snprintf(cmd, sizeof(cmd), "%s -d user2 /home 0 755", exec); run_cmd(cmd);

    // --- File Creation ---
    snprintf(cmd, sizeof(cmd), "%s -f notes.txt /home 0 664", exec); run_cmd(cmd);
    snprintf(cmd, sizeof(cmd), "%s -f secrets.txt /etc 0 600", exec); run_cmd(cmd);

    // --- Writing ---
    snprintf(cmd, sizeof(cmd), "%s -w notes.txt /home 0 2Hey", exec); run_cmd(cmd);
    snprintf(cmd, sizeof(cmd), "%s -w secrets.txt /etc 0 1TopSecret!", exec); run_cmd(cmd);

    // --- Reading full content (admin) ---
    snprintf(cmd, sizeof(cmd), "%s -rf notes.txt /home 0", exec); run_cmd(cmd);
    snprintf(cmd, sizeof(cmd), "%s -rf secrets.txt /etc 0", exec); run_cmd(cmd);

    // --- Partial read (first 5 bytes) ---
    snprintf(cmd, sizeof(cmd), "%s -r notes.txt /home 15 0", exec); run_cmd(cmd);

    // --- Read as user 1 ---
    snprintf(cmd, sizeof(cmd), "%s -rf secrets.txt /etc 1", exec); run_cmd(cmd);

    // --- Change perms to 640 ---
    snprintf(cmd, sizeof(cmd), "%s -cf secrets.txt /etc 0 640", exec); run_cmd(cmd);

    // --- Retry as user 1 ---
    snprintf(cmd, sizeof(cmd), "%s -rf secrets.txt /etc 1", exec); run_cmd(cmd);

    // --- Try as user 2 ---
    snprintf(cmd, sizeof(cmd), "%s -rf secrets.txt /etc 2", exec); run_cmd(cmd);

    // --- Move file and dir ---
    snprintf(cmd, sizeof(cmd), "%s -mvf /home/notes.txt /etc 0", exec); run_cmd(cmd);
    snprintf(cmd, sizeof(cmd), "%s -mvd /home/user2 /etc 0", exec); run_cmd(cmd);

    // --- Confirm move ---
    snprintf(cmd, sizeof(cmd), "%s -rf notes.txt /etc 0", exec); run_cmd(cmd);

    // --- Deletion ---
    snprintf(cmd, sizeof(cmd), "%s -df notes.txt /etc 0", exec); run_cmd(cmd);
    snprintf(cmd, sizeof(cmd), "%s -dd /etc/user2 0", exec); run_cmd(cmd);

    // --- Metadata ---
   // snprintf(cmd, sizeof(cmd), "%s -i", exec); run_cmd(cmd);
    //snprintf(cmd, sizeof(cmd), "%s -di", exec); run_cmd(cmd);

    printf("\n✅ Test finished.\n");
    return 0;
}
