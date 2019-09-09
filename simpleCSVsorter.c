#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#include "iohelper.h"
#include "merge.h"
#include "strhelper.h"
#include "simpleCSVsorter.h"

char DEFAULT_CATEGORIES[417] = "color,director_name,num_critic_for_reviews,duration,director_facebook_likes,actor_3_facebook_likes,actor_2_name,actor_1_facebook_likes,gross,genres,actor_1_name,movie_title,num_voted_users,cast_total_facebook_likes,actor_3_name,facenumber_in_poster,plot_keywords,movie_imdb_link,num_user_for_reviews,language,country,content_rating,budget,title_year,actor_2_facebook_likes,imdb_score,aspect_ratio,movie_facebook_likes";

//Handles the reading, parsing, sorting and outputting of the sorted data to the file
int handle_sort(Mode mode, char* sort_cat, char* input_dir, char* output_dir) {
	
	FILE* input_file = fopen(input_dir, "r");
	
	if(!input_file) {
		print_error("Could not read input file");
		return 1;
	}

	//get first line of data
	char* data = get_line(input_file);
	
	int cat_count = 0;
	char** categories = tokenize(data, ',', &cat_count);

	free(data);

	int i, j;

	for(i = 0; i < cat_count; i++) {
		char* cat = categories[i];

		if(!contains_string(DEFAULT_CATEGORIES, cat)) {
			print_error("Invalid Categories Detected");
			fprintf(stderr, "Category Found: %s\n", cat);
			return 1;
		}
	}
	
	int cat_index = find(categories, cat_count, sort_cat);

	if(cat_index < 0) {
		print_error("Invalid category selected");
		fclose(input_file);
		return 1;
	}

	int count = 0;

	data = get_line(input_file);

	int movie_size = DEFAULT_ARRAY_SIZE;

	Movie* movies = realloc(NULL, movie_size * sizeof(Movie));
	
	if(movies == NULL) {
		print_error("Cannot Allocate Memory");
		fclose(input_file);
		return 1;
	}

	while(*data != '\0') {
		Movie temp;
		
		int data_len = 0;
		char** temp_data = tokenize(data, ',', &data_len);	
		
		if(data_len > cat_count) {	//data_len exceeds cat_count
			print_error("Data columns length exceeds category count.");
			fclose(input_file);
			return 1;
		} else if(data_len > cat_count) {	//cat_count exceeds data_len
			print_error("Category count exceeds data columns count.");
			fclose(input_file);
			return 1;
		}

		temp.data = temp_data;
		temp.raw_data = data;
		temp.size = data_len;
		movies[count] = temp;

		count++;

		if(count == (movie_size - 1)) {
			movie_size += DEFAULT_ARRAY_INCREMENT;
			movies = realloc(movies, movie_size * sizeof(Movie));
			
			if(movies == NULL) {
				print_error("Cannot Allocate Memory");
				fclose(input_file);
				return 1;
			}

		}

		data = get_line(input_file);
	}

	fclose(input_file);

	int last_null = bring_null_forward(movies, cat_index, count);
	last_null = last_null >= 0 ? last_null : 0;	

	sort(movies, cat_index, last_null, count - 1);

	FILE* output_file = fopen(output_dir, "w");	

	if(!output_file) {
		print_error("Could not write to output file");
		return 1;
	}

	//Write to file
	for(i = 0; i < cat_count - 1; i++) {
		fprintf(output_file, "%s,", categories[i]);
	}

	if(cat_count > 0) {
		fprintf(output_file, "%s\n", categories[i]);
	}

	for(i = 0; i < count; i++) {
		fprintf(output_file, "%s\n", movies[i].raw_data);
	}

	fclose(output_file);

	//Freeing the data
	for(i = 0; i < count; i++) {
		Movie movie = movies[i];
		for(j = 0; j < movie.size; j++) {
			free(movie.data[j]);
		}

		free(movie.data);
		free(movie.raw_data);
	}

	free(movies);

	return 0;
}

//Gets reads and parses the execution arguments
int parse_exec_args(int argc, char **argv, Mode* mode, char** sort_cat, char** input_dir, char** output_dir) {

	int i;
	int has_c = 0, has_d = 0, has_o = 0;
	int expected_args = 1;

	for(i = 0; i < argc; i++) {
		char* arg = argv[i];

		if(is_string_equal(arg, "-c") && !has_c) {
		    has_c = i;
		    expected_args += 2;
		} else if(is_string_equal(arg, "-d") && !has_d) {
		    has_d = i;
		    expected_args += 2;
		} else if(is_string_equal(arg, "-o") && !has_o) {
		    has_o = i;
		    expected_args += 2;
		} else if(is_string_equal(arg, "-c") || is_string_equal(arg, "-d") || is_string_equal(arg, "-o")){
			print_error("Duplicate Arguments Passed");
			return 1;
		}
	}

	if(argc < expected_args) {
		print_error("Invalid Argument Count.");
		return 1;
	}

	
	if(!has_c) {
		print_error("Invalid sorting mode selected. Must specify sorting mode<c> and category<category_name>");
		return 1;
	}

	*mode = Column;
	*sort_cat = argv[has_c + 1];

	if(DEBUG) printf("Current Sort Mode is %d and sorting based on %s\n", *mode, *sort_cat);

	if(has_d) {
		*input_dir = argv[has_d + 1];

		DIR* dir = opendir(*input_dir);
		if (dir) {
    			closedir(dir);
		}
		else if (ENOENT == errno) {
			print_error("Input directory does not exist");
			return 1;
		}
		else {
			print_error("Input directory could not be opened");
			return 1;
		}
	} else {
		*input_dir = NULL;
	}

	if(has_o) {
		*output_dir = argv[has_o + 1];

		
		DIR* dir = opendir(*output_dir);
		if (dir) {
    			closedir(dir);
		}
		else if (ENOENT == errno) {
			print_error("Output directory does not exist");
			return 1;
		}
		else {
			print_error("Output directory could not be opened");
			return 1;
		}

	} else {
		*output_dir = NULL;
	}

	return 0;
}

//Main function of execution
int main(int argc, char **argv) {
	
	Mode mode;
	char* sort_cat = NULL;
	char* input_dir = NULL;
	char* output_dir = NULL;

	//parsing the input args
	int res = parse_exec_args(argc, argv, &mode, &sort_cat, &input_dir, &output_dir);

	if(res > 0) {
		return res;
	}

	int path_index = 1, path_len = DEFAULT_BUFFER_SIZE;
    	char* path = malloc(sizeof(char) * path_len);

	if(!path) {
		print_error("Cannot Allocate Memory");
		return 1;
	}

	if(input_dir) {  
		int len = strlen(input_dir), i;

		path = realloc(path, sizeof(char) * len);

		if(!path) {
			print_error("Cannot Allocate Memory");
			return 1;
		}

		for(i = 0; i < len; i++) {
			path[i] = input_dir[i];
		}

		path[i] = '\0';

		path_index = len;
	} else {
		path[0] = '.';
		path[1] = '\0';
	}

	dirent *dir_ent;

	DIR* dir = opendir(path);

	if(!dir) {
	    	print_error("Could not open the current directory as input directory");
		return 1;
	}

	dir_ent = readdir(dir);

	int is_root = 1, child_produced = 0;

	int children = 0;

	pid_t root_pid = getpid();
	
	FILE* buffer_file = stdout;

	printf("PIDS of children processes: ");
	fflush(stdout);

	while(dir_ent) {
		char* name = dir_ent->d_name;
		unsigned char type = dir_ent->d_type;

		if(type == DT_REG) {
			children++;
			pid_t pid = fork();

			if(pid == 0) {
				is_root = 0;
				children = 0;
				child_produced = 1;

				if(!ends_with(name, ".csv") || contains_string(name, "-sorted-")) {
					return 0;
				}
				
				path = append(path, "/", &path_index, &path_len);
				path = append(path, name, &path_index, &path_len);

				char* input_dir = malloc(sizeof(char) * (strlen(path) + 1));

				if(!input_dir) {
					print_error("Cannot Allocate Memory");
					return 1;
				}

				strcpy(input_dir, path);

				if(output_dir) {
					path_index = 0;
					path = append(path, output_dir, &path_index, &path_len);
					path = append(path, "/", &path_index, &path_len);
					path = append(path, name, &path_index, &path_len);
				}
				
				path_index -= 4;
				path = append(path, "-sorted-", &path_index, &path_len);
				path = append(path, sort_cat, &path_index, &path_len);
				path = append(path, ".csv", &path_index, &path_len);

				char* output_dir = malloc(sizeof(char) * (strlen(path) + 1));

				if(!output_dir) {
					print_error("Cannot Allocate Memory");
					return 1;
				}

				strcpy(output_dir, path);

				int res = handle_sort(mode, sort_cat, input_dir, output_dir);

				if(res) {
					print_error("The sorting process returned with an error");
				}

				free(input_dir);
				free(output_dir);

				return 0;
			} else {

				if(!child_produced) {
					fprintf(buffer_file, "%d", pid);
				} else {
					fprintf(buffer_file, ", %d", pid);
				}

				fflush(buffer_file);

			}

			child_produced = 1;

		} else if(type == DT_DIR && !contains(name, '.')) {
			children++;
			pid_t pid = fork();

			if(pid == 0) {
				is_root = 0;
				children = 0;
				
				if(!child_produced) {
					fprintf(buffer_file, "%d", getpid());
					child_produced = 1;
				} else {
					fprintf(buffer_file, ", %d", getpid());
				}

				fflush(buffer_file);

				path = append(path, "/", &path_index, &path_len);
				path = append(path, name, &path_index, &path_len);
				
				dir = opendir(path);
			} else {
				child_produced = 1;
			}
		}

		dir_ent = readdir(dir);
	}

	closedir(dir);
	
	free(path);

	int i, new_children = children;

	for(i = 0; i < children; i++) {
		int data;
		wait(&data);
		new_children += WEXITSTATUS(data);
	}

	children = new_children;

	if(is_root) {
		printf("\nInitial PID: %d\n", root_pid);
		printf("Total number of processes: %d\n", children);

	}

	return children;
}
