// common.h
#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <openssl/sha.h>
#include <math.h>
#include <pthread.h>

#define MAX_HASHMAP_SIZE 1024
#define HASH_FILE "hash_data.txt"
#define ARRAY_SIZE 500
#define SHA256_DIGEST_LENGTH 32

int min(int a, int b);
void Merge(int arr[], int left, int mid, int right);
void Merge_Sort(int arr[], int left, int right);
int binary_search(int arr[], int l, int r, int x);
int jump_search(int arr[], int n, int x);
int fibonacci_search(int arr[], int n, int x);
int ternary_search(int arr[], int l, int r, int x);
void load_hash_map(char *hashmap[MAX_HASHMAP_SIZE][2], int *count);
void save_hash_map(char *hashmap[MAX_HASHMAP_SIZE][2], int count);
char* hash_file(char *filename);
void bubblesort(int arr[]);
void insertionSort(int arr[], int n);
void quicksort(int *arr, int low, int high);
int count_frequency(int arr[], int n, int index, int x);

#endif 
