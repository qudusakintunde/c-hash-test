#include "hash_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void initialise_hash_table(HashTable *ht)
{
    // check if the hash table pointer is NULL
    if (ht == NULL) {   
        return;
    }
    // set all buckets to NULL to indicate they are empty
    for (int i = 0; i < HASH_TABLE_SIZE; ++i) {      
        ht->buckets[i] = NULL;
    }  
}

// This function should return a valid hash table
// index (which will be a value >= 0), or a value 
// of -1 if an error or issue is detected.
// Don't use the ERROR return type, use -1 instead

int get_bucket_for_user(int user_id)
{
    // check if the user ID is within the valid range 
     if (user_id < 1 || user_id > 2139) {  
        return -1;
    }
     return user_id % HASH_TABLE_SIZE;  //  calculate the bucket index using the hash function 
}

// This function should return a pointer to a user found
// or a value of NULL otherwise. Don't use the ERROR return type
User *find_user(const HashTable *ht, int user_id)
{
    // check if the hash table pointer is NULL
     if (ht == NULL) {
        return NULL;
    }
    //find the bucket where the user should be stored based on the user ID
    int bucket = get_bucket_for_user(user_id);
    if (bucket == -1) {
        return NULL;
    }
    // start searching from the first user in the bucket
    User *current = ht->buckets[bucket];

    
    while (current != NULL) {
        // check if the current user's ID matches the target user ID
        if (current->user_id == user_id) {
            return current;
        }
        current = current->next;  // move to the next user in the linked list
    }
    return NULL;  // return NULL if the user is not found
}

// This function should return SUCCESS upon successful addition
// of user to table (or successful updating of existing user).
// It should return ERROR for any other outcome.
int register_user(HashTable *ht, int user_id, char password[])
{
    // check if the table or password is NULL
     if (ht == NULL || password == NULL) {
        return ERROR;
    }

    //find the bucket and check if the user ID is valid
    int bucket = get_bucket_for_user(user_id);
    if (bucket == -1) {
        return ERROR;
    }
     
    //check the length of the password to ensure it does not exceed the maximum allowed length
    size_t length = 0;
    while (length <= MAX_PASSWORD_LEN && password[length] != '\0') {
        ++length;
    }
    if (length > MAX_PASSWORD_LEN) {
        return ERROR;
    }

    //update the password if the user already exists
    User *existing = find_user(ht, user_id);
    if (existing != NULL) {
        memcpy(existing->password, password, length + 1);
        return SUCCESS;
    }
    
    //Allocate memory for a new user
    User *new_user = malloc(sizeof *new_user);
    if (new_user == NULL) {
        return ERROR;
    }
    // store the user's ID and password
    new_user->user_id = user_id;
    memcpy(new_user->password, password, length + 1);

    // insert the new user at the head of the bucket
    new_user->next = ht->buckets[bucket];
    ht->buckets[bucket] = new_user;

    return SUCCESS;
}


// This function should return SUCCESS if it ascertains that 
// the password provided is a correct match for the relevant
// user in the table (as identified by 'user_id').
// It should return ERROR for any other outcome.
int is_valid_password(const HashTable *ht, int user_id, char password[])
{
    // check if the password is NULL
   if (password == NULL) {
        return ERROR;
    }
    // check the length of the password to ensure it does not exceed the maximum allowed length
    size_t length = 0;
    while (length <= MAX_PASSWORD_LEN && password[length] != '\0') {
        ++length;
    }
    if (length > MAX_PASSWORD_LEN) {
        return ERROR;
    }
    // Find the user in the hash table
    User *user = find_user(ht, user_id);
    if (user == NULL) {
        return ERROR;
    }
    // Compare the provided password with the stored password
    return strcmp(user->password, password) == 0 ? SUCCESS : ERROR;
}

void free_buckets(HashTable *ht)
{
    // check if the hash table pointer is NULL
    if (ht == NULL) {
        return;
    }
    // Go through each bucket in the hash table
    for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
        User *current = ht->buckets[i];
        
        //Free all users in the current bucket
        while (current != NULL) {
            User *next = current->next;
            free(current);
            current = next;
        }
        // set the bucket back to NULL
        ht->buckets[i] = NULL;
    }
}

// This function should return SUCCESS upon successful loading
// of data from the file. 
// It should return ERROR for any other outcome.
int load_users_from_file(HashTable *ht, char filename[])
{
    // check for invalid inputs
     if (ht == NULL || filename == NULL || filename[0] == '\0') {
        return ERROR;
    }
    // open the file for reading
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return ERROR;
    }

    char line[256];
    int result = SUCCESS;
    // read each line from the file
    while (fgets(line, sizeof line, file) != NULL) {

        // check if the line has not been cut off
        if (strchr(line, '\n') == NULL && !feof(file)) {
            result = ERROR;
            break;
        }

        // skip lines containing only whitespace
        size_t i = 0;
        while (isspace((unsigned char)line[i])) {
            ++i;
        }
        if (line[i] == '\0') {
            continue;
        }

        int user_id;
        char password[MAX_PASSWORD_LEN + 1];
        char trailing;

        // Read the ID and password, checking for invalid input and ensuring the user is registered successfully
        if (sscanf(line, " %d %40s %c", &user_id, password, &trailing) != 2 ||
            register_user(ht, user_id, password) != SUCCESS) {
            result = ERROR;
            break;
        }
    }
    // check for errors while reading or closing the file
    if (ferror(file)) {
        result = ERROR;
    }
    if (fclose(file) != 0) {
        result = ERROR;
    }

    // clear any partially loaded users if an error occurred
    if (result == ERROR) {
        free_buckets(ht);
    }
    return result;
}