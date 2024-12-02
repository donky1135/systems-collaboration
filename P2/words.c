#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdarg.h>

#define PATH_MAX 4096

#define BUFSIZE 16

typedef struct{
	char *word;
	int n;
} in_map;

int is_space(char c){ return (c != '-') && (c != '\'') && (c < 'A' || ('Z' < c && c < 'a') || ('z' < c)); }

/*
char to_lower(char c){ return (('A' <= c) && (c <= 'Z'))? c + ('a' - 'A') : c; }
char *lower_cpy(char *dst, const char *src){
    strcpy(dst, src);
    char *p = dst;
    while(*p != '\0'){
        *p = to_lower(*p);
        p++;
    }
    return dst;
}
int lower_cmp(char *s1, char *s2){
    int len = 0;
    while(s1[++len] != '\0');
    char *buf1 = (char *)malloc(sizeof(char) * (len + 1));
    len = 0;
    while(s2[++len] != '\0');
    char *buf2 = (char *)malloc(sizeof(char) * (len + 1));
    int out = strcmp(lower_cpy(buf1, s1), lower_cpy(buf2, s2));
    free(buf1);
    free(buf2);
    return out;
}
*/

int cmp_w(const void* m1, const void* m2) { return strcmp((*(in_map**)m1)->word, (*(in_map**)m2)->word); }
int cmp_n(const void* m1, const void* m2) {
    if ((*(in_map**)m1)->n == (*(in_map**)m2)->n) {
        /*
        char *s1 = (*(in_map **)m1)->word;
        char *s2 = (*(in_map **)m2)->word;
        if(lower_cmp(s1, s2) == 0) return strcmp(s1, s2);
        else return lower_cmp(s1, s2);
        */
        return strcmp((*(in_map**)m1)->word, (*(in_map**)m2)->word);
    }
    else return -1 * ((*(in_map**)m1)->n - (*(in_map**)m2)->n);
}

typedef struct{
	int len;
	int num;
	in_map **m;
} map;

void free_map(map *m){
    for(int i = 0; i < m->num; i++){
        free((m->m)[i]->word);
        free((m->m)[i]);        
    }
    free(m->m);
    free(m);
}

void sort_num(map *this){ qsort(this->m, (size_t)this->num, sizeof(in_map *), cmp_n); }
void sort_word(map *this){ qsort(this->m, (size_t)this->num, sizeof(in_map *), cmp_w); }

int find(map *this, char *word){
    if(this->num == 0) return -1;
    sort_word(this);
    int l = 0, r = this->num - 1, mid = l + (r - l) / 2;
        while(l <= r){
            int cmp = strcmp(((this->m)[mid])->word, word);
            switch(cmp){
                case 0:{
                    return mid;
                }
                case -1:{
                    l = mid + 1;
                    mid = l + (r - l) / 2;
                    break;
                }
                case 1:{
                    r = mid - 1;
                    mid = l + (r - l) / 2;
                    break;
                }
            }
        }
    return -1;
}

int add(map *this, char *word){
    int index = find(this, word);
    if(index == -1){
        if(this->num == this->len){
            this->len *= 2;
            this->m = (in_map **)realloc(this->m, sizeof(in_map *) * this->len);
        }
        in_map *new_word = (in_map *)malloc(sizeof(in_map));
        new_word->word = word;
        new_word->n = 1;
        (this->m)[this->num] = new_word;
        (this->num)++;
        sort_word(this);
        return 1;
    }else{
        (((this->m)[index])->n)++;
        return 0;
    }
}

int file_count(char *);
int dir_count(char *);
int is_txt(char *);

map *word_cnt;

int words(int argc, char **argv) {
    word_cnt = (map *)malloc(sizeof(map));
	word_cnt->len = 16;
	word_cnt->num = 0;
	word_cnt->m = (in_map **)malloc(sizeof(in_map *) * word_cnt->len);

    char *filename;
    struct stat buf_st;
    for(int i = 0; i < argc; i++){
        filename = argv[i];
        if(filename[0] == '.') continue;
        char path[] = "./";
        if(!stat(filename, &buf_st)){
            if(S_ISDIR(buf_st.st_mode)) dir_count(filename);
            else if(S_ISREG(buf_st.st_mode)) file_count(filename);
        }
    }

	sort_num(word_cnt);
	for(int i = 0; i < word_cnt->num; i++){
        write(1,((word_cnt->m)[i])->word,strlen(((word_cnt->m)[i])->word));
        int count = ((word_cnt->m)[i])->n;
        int log = 0;
        while(count > 0){
            count /= 10;
            log++;
        }
        char* buff = malloc(sizeof(char)*(3 + log));
        sprintf(buff," %d\n",((word_cnt->m)[i])->n);
        write(1,buff,3+log);
    	// printf("%s %d\n", ((word_cnt->m)[i])->word, ((word_cnt->m)[i])->n); //need to change to write to stdio
        free(buff);
    
    }
    free_map(word_cnt);
    return 0;
}

int file_count(char *filename){
    int fd = open(filename, O_RDONLY);

	int word_len_max = BUFSIZE;
	char *word = (char *)malloc(sizeof(char) * (word_len_max + 1));
	int in_word = 0;
	int len_word = 0;
	int bytes;

    char buf[BUFSIZE];

	while((bytes = read(fd, buf, BUFSIZE)) > 0){
		for(int i = 0; i < bytes; i++){
			if(is_space(buf[i])){
				if(in_word){
					if(word[len_word - 1] == '-') len_word--;
					word[len_word] = '\0';
                    word = (char *)realloc(word, sizeof(char) * (len_word + 1));
					if(!add(word_cnt, word)) free(word);
					
                    word_len_max = BUFSIZE;
					word = (char *)malloc(sizeof(char) * (word_len_max + 1));
					in_word = 0;
					len_word = 0;
				}
			}else{
                if(buf[i] == '-'){
                    if(!in_word) continue;
                    else if(word[len_word - 1] == '-'){
                        len_word--;
                        word[len_word] = '\0';
                        word = (char *)realloc(word, sizeof(char) * (len_word + 1));
                        if(!add(word_cnt, word)) free(word);
                        
                        word_len_max = BUFSIZE;
                        word = (char *)malloc(sizeof(char) * (word_len_max + 1));
                        in_word = 0;
                        len_word = 0;
                        
                        continue;
                    }
                }
                if(len_word == word_len_max){
					word_len_max *= 2;
					word = (char *)realloc(word, sizeof(char) * (word_len_max + 1));
				}
				word[len_word] = buf[i];
				in_word = 1;
				len_word++;
			}
		}
	}
	if(in_word){
		if(word[len_word - 1] == '-') len_word--;
		word[len_word] = '\0';
		if(!add(word_cnt, word)) free(word);
	} else {
        free(word);
    }
    close(fd);
    return 0;
}

int dir_count(char *dirname){
    DIR *dir = opendir(dirname);
    if(!dir){
        perror(dirname);
        exit(EXIT_FAILURE);
    }

    struct dirent *ent;
    struct stat buf_st;
    char pname[PATH_MAX];

    while((ent = readdir(dir))){
        char *dname = ent->d_name;
        if(dname[0] == '.') continue;

        strcpy(pname, dirname);
        strcat(pname, "/");
        strcat(pname, dname);

        if(!stat(pname, &buf_st)){
            if(S_ISDIR(buf_st.st_mode)){
                dir_count(pname);
            }
            else if(S_ISREG(buf_st.st_mode) && is_txt(pname)){
                file_count(pname);
            }
        }       
    }
    closedir(dir);
    return 0;
}

int is_txt(char *filename){
    int len = strlen(filename);
    return ((len > 4) && (strcmp(&filename[len-4], ".txt") == 0));
}

int main(int argc, char **argv){
    words(argc, argv);
}