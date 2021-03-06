#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>

#define CURSOR_UP "\033[1A"
#define CURSOR_DOWN "\033[1B"
#define CURSOR_RIGHT "\033[1C"
#define CURSOR_LEFT "\033[1D"
#define CURSOR_SAVE "\033[s"
#define CURSOR_RESTORE "\033[u"
#define ALT_BUFFER "\033[?1049h\033[H"
#define PRESERVED_SCREEN "\033[?1049l"
#define CLEAR_LINE "\033[2K"
typedef struct line { 
  int file_offset;
  int begin_edit; 
  char* text; 
  char* status;
  struct line* next;
} line;
typedef struct termios termios;
typedef struct winsize winsize;

int init(line*);
int cleanup(line*);
int fill_buffers(int, int, line*);
int print_buffers(line*);
int open_screen_buffer(termios*);
int open_preserved_screen(termios*);
int detect_keypress(int*, int*,line**,line**,FILE*);
int get_cursor(int*,int*,line*,line**);
line* get_previous(FILE*, int);
line* get_next(FILE*, int);

static winsize global_win;

int main(){
  int cursor_row, cursor_col, read_line;
  struct termios term;
  struct winsize window;
  //ioctl(STDOUT_FILENO, TIOCGWINSZ, &window);
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &global_win);

  line* first_line = (line*)malloc(sizeof(line));
  line* changed_lines;
  line* current_line;

  init(first_line);

  int map = open("./map",O_RDWR);
  FILE* fstream = fdopen(map,"r");
  fill_buffers(map,0,first_line);
  open_screen_buffer(&term);  
  print_buffers(first_line);

  while (detect_keypress(&cursor_row,&cursor_col,&first_line,&current_line,fstream));

  open_preserved_screen(&term);

  cleanup(first_line);
  close(map);
}

int get_cursor(int* row, int* col, line* first_line, line** current_line){
  printf("\033[6n");
  int temp;
  temp = getchar();//\033
  temp = getchar();//]
  *row = getchar()-'0';//row
  temp = getchar()-'0';
  while (temp >= 0 && temp <= 9){
    *row = 10 * *row + temp; 
    temp = getchar()-'0';
  }
  //; divider
  *col = getchar()-'0';//col
  temp = getchar()-'0';
  while (temp >= 0 && temp <= 9){
    *col = 10 * *col + temp; 
    temp = getchar()-'0';
  }
  while (temp != 'R'-'0'){
    temp = getchar()-'0';
  }
  //update current_line
  int i;
  for (i=1;i<*row;i++){
    first_line = first_line->next;
  }
  *current_line = first_line;
}

int detect_keypress(int* cursor_row, int* cursor_col, line** first_line_ptr, line** current_line, FILE* fstream){
  int key;
  line* first_line = *first_line_ptr;
  key = getchar();
  if (key == 17){//ctrl+q
    return 0;
  }
  else if (key == '\033'){
    if (getchar() == '['){
      key = getchar();
      if (key == 65){
	if (*cursor_row == 1 && first_line->file_offset > 1){//up
	  line* new_first = get_previous(fstream,first_line->file_offset);
	  new_first->next = first_line;
	  line* last;
	  while (first_line->next != NULL){
	  	last = first_line;
	  	first_line = first_line->next;
	  }
	  last->next = NULL;
	  free(first_line->text);
	  free(first_line->status);
	  free(first_line);
	  *first_line_ptr = new_first;
	  first_line = *first_line_ptr;
	  printf(CURSOR_SAVE);
	  print_buffers(first_line);
	  printf(CURSOR_RESTORE);
	}
	printf(CURSOR_UP);
	get_cursor(cursor_row, cursor_col,first_line,current_line);
      }
      else if (key == 66){
	if (*cursor_row == global_win.ws_row-1){
	  (*current_line)->next = get_next(fstream,(*current_line)->file_offset);
	  free((*first_line_ptr)->text);
	  free((*first_line_ptr)->status);
	  free(*first_line_ptr);
	  (*first_line_ptr) = first_line->next;
	  first_line = (*first_line_ptr);
	  printf(CURSOR_SAVE);
	  
	  print_buffers(first_line);
	  printf(CURSOR_RESTORE);
	}
	else{
		printf(CURSOR_DOWN);
	}
	get_cursor(cursor_row, cursor_col,first_line,current_line);
	  }
      else if (key == 67){
	printf(CURSOR_RIGHT);
	get_cursor(cursor_row, cursor_col,first_line,current_line);
      }
      else if (key == 68){
	printf(CURSOR_LEFT);
  	get_cursor(cursor_row, cursor_col,first_line,current_line);
      }
    }
  }
  else if (*cursor_col-1 <= (*current_line)->begin_edit ||
  	*cursor_col-1 > strlen((*current_line)->text)-1){
  	//editing something that shouldn't be edited
  	return 1;
  }
  else if (key == 127){//backspace
  	//get_cursor(cursor_row, cursor_col,first_line,current_line);
  	printf(CURSOR_LEFT);
   	printf(CURSOR_SAVE);
    line* cur = *current_line;
    strcpy(&cur->text[*cursor_col-2],&cur->text[*cursor_col-1]);
    printf(CLEAR_LINE);
    printf("\r%s",cur->text);//clear line
    printf(CURSOR_RESTORE);//reset position
    *cursor_col = *cursor_col - 1;
  }
  else{
  	printf(CURSOR_RIGHT);
  	printf(CURSOR_SAVE);
    char temp[global_win.ws_col];
    temp[0] = (char)key;
    temp[1] = 0;
    line* cur = *current_line;
    strcat(temp,&cur->text[*cursor_col-1]);
    strcpy(&cur->text[*cursor_col-1],temp);
    printf(CLEAR_LINE);
    printf("\r%s",cur->text);//clear line
    printf(CURSOR_RESTORE);//reset position
  	*cursor_col = *cursor_col + 1;
  }
}

int fill_buffers(int file, int start_read, line* first_line){
  FILE* fstream = fdopen(dup(file),"r");
  size_t buff_size = sizeof(first_line->text); 
  first_line->file_offset = 1;
  //int read = (int)getline(&first_line->text, &buff_size, fstream);
  getline(&first_line->text, &buff_size, fstream);
  while(first_line->next != NULL){
    first_line = first_line->next;
    first_line->file_offset = (int)ftell(fstream);
    //read = (int)getline(&first_line->text, &buff_size, fstream);
    getline(&first_line->text, &buff_size, fstream);
    strcpy(first_line->status,first_line->text);
    if (first_line->file_offset == -1){
      printf("%s\n",strerror(errno));
    }
    first_line->begin_edit = strlen(first_line->text)-1;
    while (first_line->text[first_line->begin_edit] != '/'){
    	first_line->begin_edit = first_line->begin_edit - 1;
    }
  }
  fclose(fstream);
}

int print_buffers(line* first_line){
  //int i=1;
  while (first_line != NULL){
    //printf("%*s\r%d%s",window->ws_col,first_line->status,first_line->file_offset,first_line->text);
    printf("%*s\r%s",global_win.ws_col,first_line->status,first_line->text);
    first_line = first_line->next;
    //i++;
  }
  //printf("%*s",global_win.ws_col,"Control + Q to quit");
}

int open_screen_buffer(termios* term){
  printf(ALT_BUFFER);
  tcgetattr(STDIN_FILENO,term);
  term->c_lflag = ~(ICANON|ECHO);
  tcsetattr(STDIN_FILENO,TCSANOW,term);
}

int open_preserved_screen(termios* term){
  printf(PRESERVED_SCREEN);
  term->c_lflag = (ICANON|ECHO);
  tcsetattr(STDIN_FILENO,TCSANOW,term);
}

int init(line* line_node){
  int i;
  for (i=1; i<global_win.ws_row - 1; i++){
    line_node->text = (char*)malloc(global_win.ws_col);
    line_node->status = (char*)malloc(global_win.ws_col);
    line_node->next = (line*)malloc(sizeof(line));
    line_node = line_node->next;
  }
  return 0;
}

line* get_previous(FILE* fstream, int next_line){//get previous line
  //FILE* fstream = fdopen(dup(map),"r");
  line* first = (line*)malloc(sizeof(line));
  size_t buff_size = sizeof(first->text); 
  first->text = (char*)malloc(sizeof(global_win.ws_col));
  first->status = (char*)malloc(sizeof(global_win.ws_col));
  fseek(fstream,next_line,SEEK_SET);
  char temp;
  int off;
  while (temp != '\n' && off > 0){
    fseek(fstream,-2,SEEK_CUR);
    off = ftell(fstream);
    temp = (char)fgetc(fstream);
  }
  first->file_offset = ftell(fstream);
  getline(&first->text, &buff_size, fstream);
  //fclose(fstream);
  return first;
}

line* get_next(FILE* fstream, int previous_line){//get next line
  //FILE* fstream = fdopen(dup(map),"r");
  line* next = (line*)malloc(sizeof(line));
  size_t buff_size = sizeof(next->text);
  next->text = (char*)malloc(sizeof(global_win.ws_col));
  next->status = (char*)malloc(sizeof(global_win.ws_col));
  fseek(fstream,previous_line,SEEK_SET);
  getline(&next->text, &buff_size, fstream);
  next->file_offset = ftell(fstream);
  getline(&next->text, &buff_size, fstream);
  next->next = NULL;
  //fclose(fstream);
  return next;
}

int cleanup(line* line_node){
  while (line_node->next){
    free(line_node);
    free(line_node->text);
    free(line_node->status);
    line_node = line_node->next;
  }
  return 0;
}