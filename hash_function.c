#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define UINT32_SIZE 4294967295
#define UINT8_SIZE 127
#define SEED 2863311530
#define STR_SIZE 10


// array的linked list的節點結構
typedef struct node {
    char str[STR_SIZE];
    struct node* next;
}Node;

// 將原本有UINT32_SIZE的數字轉為n(array size)
uint32_t scaling(uint32_t h, int arr_size)
{
    return (uint32_t)(((float)h / UINT32_SIZE) * arr_size);
}

// hash function
uint32_t hash_function(const void *str, int len, int n, int is_search)
{
    // 參數(找到較佳的performance即可)
    const uint32_t c1 = 1540483477;
    const uint32_t c2 = 110416972;
    const int r1 = 24;
    const int r2 = 27;


    // 不同長度有不同的開始(h)
    uint32_t len_shifted = 0; // 記得令為0，不然會有內存亂碼
    for (int i = 0; i < 4 ; i++) 
    {
        len_shifted |= (len << (8 * i));
    }

    uint32_t h = SEED ^ len_shifted;
    

    unsigned char * data = (unsigned char *)str;
    // 將原本的字元拆成每4個bytes做運算，算完的h在後面繼續使用，以達到均勻性
    while(len >= 4)
    {
        uint32_t k = *(uint32_t*)data;

        // 位元位移和相乘常數
        k *= c1;
        k ^= k >> r1;
        k *= c1;

        k *= c2;
        k ^= k >> r2;
        k *= c1;


        h *= c1;
        h ^= k;

        h *= c2;
        h ^= k;


        data += 4;
        len -= 4;
    }

    
    // 處理剩下的字元(小於4)
    uint32_t remain = 0;
    while(len > 0)
    {
        remain |= data[len - 1] << ((len - 1) * 8);
        len -= 1;
    }
    
    h ^= remain;
    h *= c1;
    
    // 最後做一些調整讓每個bit皆可被運算到
    h ^= len_shifted;

    h ^= h >> 13;
    h *= c1;
    h ^= h >> 15;
    h *= c1;
    h ^= h >> 17;

    if(!is_search)
        printf("(unscaling = %u\t", h);

    h = scaling(h, n);
    
    if(!is_search)
        printf("scaling = %u)\n",h);

    return h;
} 

// 隨機生成字串(回傳二維陣列)
char **rand_generate_string(int x)
{
    // 字串array
    char **all_string = (char **)malloc(x * sizeof(char *));
    for (int i = 0; i < x; i++) 
    {
        all_string[i] = (char *)malloc(STR_SIZE * sizeof(char));
    }

    // 隨機種子
    srand(time(NULL));
    for(int i = 0; i < x; i++)
    {   
        int max = STR_SIZE, min = 1;
        int str_len = rand() % ((max + 1) - min) + min; // 1 ~ STR_SIZE char

        // 隨機字串(加1為了結束符號'\0')
        char str[STR_SIZE + 1] = {};
        for(int j = 0; j < str_len; j++)
        {
            int max = 122, min = 48; // ASCII code
            int flag = 1; // is number or alph
            while(flag)
            {
                int c = rand() % ((max + 1) - min) + min; 
                // 限制在0~9和A到Z和a~z之間
                if((c >= 48 && c <= 57) || (c >= 65 && c <= 90) || (c >= 97 && c <= 122))
                {
                    str[j] = c;
                    flag = 0;
                }
            }
        }
        strcpy(all_string[i], str);
    }
    return all_string;
}

// 加入新的一筆資料
void add_newData(int h, char* data, Node** array)
{
    Node* new_data = (Node*)malloc(sizeof(Node));
    strcpy(new_data -> str, data);
    new_data -> next = NULL;

    if(array[h] == NULL)
    {
        array[h] = new_data;
    }
    // 發生碰撞
    else
    {
        Node* curr_node = array[h];
        while(curr_node -> next != NULL)
        {
            curr_node = curr_node -> next;
        }
        curr_node -> next = new_data;
    }
}

// 印出陣列
void print_arr(Node** array, int n, int* coll)
{
    for(int i = 0; i < n; i++)
    {
        if(array[i] != NULL)
        {
            Node* curr = array[i];
            while(curr != NULL)
            {
                printf("array[%d] = %s", i, curr -> str);
                curr = curr -> next;
                if(curr != NULL)
                {
                    *coll += 1;
                    printf(" -> ");
                }
            }
            printf("\n");
        }
    }
}

// 印出x筆資料各個的hash值
void print_record(int* record, int x)
{
    printf("hash_record = [");
    for(int i = 0; i < x; i++)
    {
        printf("%d,", record[i]);
    }
    printf("]\n");
}

int str_compare(char *str1, char *str2) 
{
    int i = 0;
    while (str1[i] == str2[i]) 
    {
        if (str1[i] == '\0' || str2[i] == '\0')
            break;
        i++;
    }
    if (str1[i] == '\0' && str2[i] == '\0')
        return 1;
    else
        return 0;
}

// 找字串的hash值
void search_hash(Node** array, char* data, int n)
{
    uint32_t h; // 陣列的index(hash值)
    // hash function
    h = hash_function(data, strlen(data), n, 1);
    if(array[h] != NULL)
    {
        Node* curr = array[h];
        int link_num = 0;
        while(!(str_compare(data, curr->str)))
        {
            curr = curr -> next;
            link_num++;
        }
        printf("String : \"%s\" is at linked list number [%d] of the array[%d].\n", data, link_num + 1, h);
    }
    else
    {
        printf("Not in the array\n");
    }

}



int main() 
{
    // input
    int n; // array size
    int x; // data size
    printf("Input n and x: ");
    scanf("%d%d", &n, &x);

    // 字串array
    Node* array[n];
    for (int i = 0; i < n; i++) 
    {
        array[i] = NULL;
    }

    // 自動生成字串陣列
    char **test_str = rand_generate_string(x);

    int coll = 0; // 碰撞次數

    int record[x]; // 記錄每筆資料的h
    int h_curr = 0;

    clock_t start, end; // 紀錄時間
   
    start = clock(); // 開始時間

    // 跑每筆資料
    for(int i = 0; i < x; i++)
    {
        // 每個生成的字串
        printf("NO.%d string : ", i + 1);
        puts(test_str[i]);

        uint32_t h; // 陣列的index(hash值)
        // hash function
        h = hash_function(test_str[i], strlen(test_str[i]), n, 0);
        
        record[h_curr++] = h;

        // 加到陣列裡
        add_newData(h, test_str[i], array);
    }
    end = clock(); // 結束時間

    printf("--------------------\n");

    // 時間
    printf("%d data take %.3f second.\n", x, (double)(end - start) / CLOCKS_PER_SEC );
    printf("--------------------\n");

    // 陣列
    print_arr(array, n, &coll);
    printf("--------------------\n");

    // 各個h值
    print_record(record, x);
    printf("--------------------\n");

    // 碰撞次數
    printf("Collision number = %d\n", coll);
    printf("--------------------\n");

    // 找字串的hash值 (示範範例:測試資料中的第一筆)
    search_hash(array, test_str[0], n);

    system("pause");
    return 0;
}