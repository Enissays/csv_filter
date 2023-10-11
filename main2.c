#include<stdio.h>
#include<stdbool.h>
#include<string.h>
#include<stdlib.h>

#define MAX_TEXT 1000
#define DELIMITER ","
#define MAX_FILTERS 7
#define MAX_GROUPS 7


struct Filter
{
    int column_index;
    char value[MAX_TEXT];
    char operator[2];
};

// Unfortunately, strtok skips empty tokens (i.e. consecutive delimiters), so we need to use this function instead
char *strtok_new(char * string, char const * delimiter)
{
    static char *source = NULL;
    char *p, *ret = 0;
    if (string != NULL)
        source = string;
    if (source == NULL)
        return NULL;
    if ((p = strpbrk(source, delimiter)) != NULL)
    {
        *p = 0;
        ret = source;
        source = ++p;
    }
    return ret;
} 

// Function that will go the specific column of the line and compare it with the filter
bool line_comp(char temp_line[], struct Filter filter)
{
    // Split the line by commas and gets tokens from the line
    char line[MAX_TEXT];
    strcpy(line, temp_line);
    char *token = strtok_new(line, ",");

    // Loop through the tokens to get the one we're looking for
    int column_index = 0;
    while (token != NULL)
    {
        // If the column is the one we're looking for, compare it with the filter
        if (column_index == filter.column_index)
        {
            // Remove the \n from the token
            token[strcspn(token, "\n")] = 0;

            // Compare the token with the filter
            if (strcmp(filter.operator, "=") == 0)
                return strcmp(token, filter.value) == 0;
            else if (strcmp(filter.operator, "!=") == 0)
                return strcmp(token, filter.value) != 0;
            else if (strcmp(filter.operator, ">") == 0)
                return strcmp(token, filter.value) > 0;
            else if (strcmp(filter.operator, "<") == 0)
                return strcmp(token, filter.value) < 0;
            else if (strcmp(filter.operator, ">=") == 0)
                return strcmp(token, filter.value) >= 0;
            else if (strcmp(filter.operator, "<=") == 0)
                return strcmp(token, filter.value) <= 0;
        }

        // Get the next token
        token = strtok_new(NULL, ",");
        column_index++;
    }
    return false;
}

// Apply the filters on the file, by going through each line and checking if it matches the filters
void apply_filter(FILE *fp, FILE *output, struct Filter* filters[], int f_count[], int group_count)
{
    int row_nb = 0;

    // Place the file pointer at the beginning of the file
    fseek(fp, 0, SEEK_SET);


    // Print the header of the file first 
    char header[MAX_TEXT];
    fgets(header, MAX_TEXT, fp);
    fprintf(output, "%s", header);

    // Split the header by commas and gets tokens from the header
    char *token = strtok(header, ",");
    int column_count = 0;
    while (token != NULL)
    {
        // Copy the token in the array of headers
        token = strtok(NULL, ",");
        column_count++;
    }

    // Go through each line of the file
    char line[MAX_TEXT];
    while (fgets(line, MAX_TEXT, fp))
    {
        bool value_match = true;
        // Split the line by commas and gets tokens from the line

        // Go through each group of filters and see if one of them matches the line
        for (int j=0; j<group_count; j++)
        {
            int column_index = 0;


            // Loop through the filters to check if a column is being filtered
            for (int i = 0; i < f_count[j]; i++)
                {
                    struct Filter filter = filters[j][i];
                    // Check if the value matches the filter
                    value_match = line_comp(line, filter);

                    // If there's no match, leave the loop
                    if (!value_match)
                        break;
            }

            token = strtok(NULL, ",");
            column_index++;
            if (value_match)
                break;
        }
        // If the line matches the filter, print it to the output file
        if (value_match)
        {
            fprintf(output, "%s", line);
            row_nb++;
        }
    }

    // Print the number of rows that match the filter
    printf("\n%d rows match the filter\n", row_nb);
}


// Compares the input with the headers of the file
int compare(char input[], char headers[][MAX_TEXT], int header_count)
{

    // Compare the input with each header on a loop to check if it's valid
    for (int i = 0; i < header_count; i++)
    {
        char *h = headers[i];

        if (strcmp(input, headers[i]) == 0)
            return i;
    }
    return -1;
}

// Compare the filter with the valid filters
int compare_filter(char filter[])
{
    char *valid_filters[] = {"=", "!=", ">", "<", ">=", "<="};
    // Remove the \n from the input
    filter[strcspn(filter, "\n")] = 0;

    // Compare the input with each header on a loop to check if it's valid
    for (int i = 0; i < 6; i++)
    {
        char *h = valid_filters[i];

        if (strcmp(filter, valid_filters[i]) == 0)
            return 0;
    }
    return 1;
}

void store_header(FILE *fp, char headers[][MAX_TEXT], int *header_count)
{
    // Print the header of the file first 
    char header[MAX_TEXT];
    fgets(header, MAX_TEXT, fp);

    // Split the header by commas and gets tokens from the header
    char *token = strtok(header, ",");
    while (token != NULL)
    {
        // Print each token individually
        printf("%s ", token);

        // Store the token in the array of headers
        strcpy(headers[*header_count], token);
        token = strtok(NULL, ",");

        // Increment the counter
        (*header_count)++;
    }
}

// Ask the user for the filters he wants to apply
// Inputted filters will be separated by commas, split these filters and store them in an array of strings first to determine how many filters are there, and realloc the filters table by that size, then for each array element, split it by spaces to get the column index, the operator and the value
void input_filters(struct Filter* filters[MAX_GROUPS], int filter_count[], char header[100][MAX_TEXT], int header_count, int *group_count)
{
    char input[MAX_TEXT];
    while (strcmp(input, "exit") != 0 && *group_count < MAX_GROUPS) {
        printf("\nWhat filters would you like to apply ? (E.G : a = b, c > d) (exit to stop)\n");
        fgets(input, MAX_TEXT, stdin);
        // Remove the \n from the input
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "exit") == 0)
            break;

        // Split the input by commas and gets tokens from the input, store them in an array of strings
        char *token = strtok(input, DELIMITER);
        char *filter_tokens[MAX_FILTERS];
        while (token != NULL)
        {
            if (filter_count[*group_count] == MAX_FILTERS)
            {
                printf("Maximum number of filters reached.\n");
                return;
            }
            // Store the token in the array of headers
            filter_tokens[filter_count[*group_count]] = token;
            token = strtok(NULL, DELIMITER);
            filter_count[*group_count]++;
        }

        // Realloc the filters table by the number of filters
        filters[*group_count] = malloc (filter_count[*group_count] * sizeof(struct Filter));
        
        // Loop through the filters to split them by spaces and get the column index, the operator and the value
        for (int i = 0; i < filter_count[*group_count]; i++)
        {
            // Split the filter by spaces and gets tokens from the filter
            token = strtok(filter_tokens[i], " ");
            int token_count = 0;
            char *filter[3];
            while (token != NULL)
            {
                // Store the token in the array of headers
                filter[token_count] = token;
                token = strtok(NULL, " ");
                token_count++;
            }

            // Check if the filter is valid
            if (token_count != 3)
            {
                printf("Invalid filter\n");
                return;
            }

            // Check if the column index is valid
            int column_index = compare(filter[0], header, header_count);
            if (column_index == -1)
            {
                printf("Invalid column index\n");
                return;
            }

            // Check if the operator is valid
            if (compare_filter(filter[1]) == 1)
            {
                printf("Invalid operator\n");
                return;
            }

            // Store the filter in the filters table
            struct Filter f;
            f.column_index = column_index;
            strcpy(f.operator, filter[1]);
            strcpy(f.value, filter[2]);
            filters[*group_count][i] = f;
        }

        printf("Defined %d filters successfully\n", filter_count[(*group_count)]);
        (*group_count)++;

    }
}

int main()
{

    // ask the user for the path of the file
    char CSV_PATH[MAX_TEXT];
    printf("What's the path of the file you'd like to filter? (Enter the word \"exit\" to escape.) (main 2)\n");
    scanf("%s", CSV_PATH);
    getchar();
    if (strcmp(CSV_PATH, "exit") == 0)
        return 1;

    // Open the necessary file to read from
    FILE *fp = fopen(CSV_PATH, "r");

    // Check if the file is opened successfully
    if (fp == NULL)
    {
        printf("Error opening file\nUnfound file, are you sure you specified the path correctly?\n");
        main();
    }

    // Build the header of the file in an array of strings
    char header[100][MAX_TEXT];
    int header_count = 0;
    store_header(fp, header, &header_count);

    // Initiate the array of filters structure 
    struct Filter* filters[7];
    int filter_count[MAX_GROUPS] = {0};

    int column_index = -1;
    int group_count = 0;
    input_filters(filters, filter_count, header, header_count, &group_count);
    // Open an output file to write the filtered data
    FILE *output = fopen("output.csv", "w");
    apply_filter(fp, output, filters, filter_count, group_count);

    // Close the files
    fclose(fp);
    fclose(output);
}
