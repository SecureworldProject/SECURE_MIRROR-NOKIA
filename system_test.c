/////  FILE INCLUDES  /////

#include "system_test.h"
#include "logic.h"
#include "keymaker.h"
#include "wchar.h"




/////  DEFINITIONS AND GLOBAL VARIABLES  /////

struct Test {
	// All streams refer to the wrapper (eg. input to the wrpper or output to the wrapper)
	uint8_t* input_stream;
	uint8_t* desired_output_stream;
	uint8_t* output_stream;
	enum TestVerdict verdict;
};

enum TestVerdict {
	TEST_VERDICT_NOT_DONE,
	TEST_VERDICT_FAIL,
	TEST_VERDICT_PASS,
	TEST_VERDICT_NOT_APPLICABLE_OK,
	TEST_VERDICT_ABORT_OK
};
static const char* TEST_VERDICT_STRINGS[] = { "NOT DONE", "FAIL", "PASS", "N/A (OK)", "ABORT (OK)"};


// Paths to the test files
#define TEST_C_FOLDER_PATH L"C:\\Users\\Pepito\\TEST\\"		//L"C:\\secureworld\\"
#define TEST_M_FOLDER_PATH L"E:\\"		//L"\\\\?\\E:\\"						//L"M:\\secureworld\\"
//const WCHAR* TEST_SMALL_FILE_NAME			= L"TEST_SMALL.txt";
//const WCHAR* TEST_BIG_DECIPHERED_FILE_NAME	= L"TEST_BIG_DECIPHERED.txt";
//const WCHAR* TEST_BIG_CLEARTEXT_FILE_NAME	= L"TEST_BIG_CLEARTEXT.txt";
//const WCHAR* TEST_BIG_CIPHERED_FILE_NAME	= L"TEST_BIG_CIPHERED.txt";

// Stream definitions and variables (filled in initTestStreams())
#define SMALL_STREAM_BUFFER_SIZE (DWORD)500
DWORD big_stream_buffer_size = 0;
uint8_t* small_stream = NULL;
uint8_t* big_deciphered_stream = NULL;
uint8_t* big_cleartext_stream = NULL;
uint8_t* big_ciphered_stream = NULL;
uint8_t* all_test_streams[4] = { NULL, NULL, NULL, NULL };

/*
Already defined enums that will be used here:

enum AppType {
	ANY,
	BROWSER,
	MAILER,
	BLOCKED,
	TEST
};
const char* APP_TYPE_STRINGS[] = { "ANY", "BROWSER", "MAILER", "BLOCKED", "TEST"};

enum IrpOperation {
	IRP_OP_READ,
	IRP_OP_WRITE
};
#define NUM_IRP_OPERATIONS 2
const char* IRP_OPERATION_STRINGS[] = { "READ", "WRITE" };

enum Operation {
	NOTHING,
	CIPHER,
	DECIPHER
};
const char* OPERATION_STRINGS[] = { "NOTHING", "CIPHER", "DECIPHER" };
*/

enum OperationPosition {
	INSIDE_MARK = 0,
	INSIDE_AND_OUTSIDE_MARK,
	OUTSIDE_MARK
};
static const char* OPERATION_POSITION_STRINGS[] = { "INSIDE_MARK", "INSIDE_AND_OUTSIDE_MARK", "OUTSIDE_MARK" };

enum StreamLevel {
	SMALL = 0,
	BIG_DECIPHERED,
	BIG_CLEARTEXT,
	BIG_CIPHERED
};
static const char* FILE_SIZE_AND_LEVEL_STRINGS[] = { "SMALL  (0)", "BIG (-1)", "BIG  (0)", "BIG (+1)" };


// Dimensions in the array are (from the leftmost square bracket to the rightmost one):
//	1st dimension:	IrpOperation  (0=IRP_OP_READ, 1=IRP_OP_WRITE)
//	3rd dimension:	OperationPosition  (0=INSIDE_MARK, 1=INSIDE_AND_OUTSIDE_MARK, 2=OUTSIDE_MARK)
//	2nd dimension:	Operation  (0=NOTHING, 1=CIPHER, 2=DECIPHER)
//	4th dimension:	StreamLevel  (0=SMALL, 1=BIG_DECIPHERED, 2=BIG_CLEARTEXT, 3=BIG_CIPHERED)
static struct Test tests_array[2][3][3][4] = { 0 };

//struct Tuple test_operative = { .app_type = TEST, .on_read = NOTHING, .on_write = NOTHING };




/////  FUNCTION PROTOTYPES  /////

DWORD initTestStreams();
void freeTestStreams();
void unitTest(enum IrpOperation irp_op, enum Operation ciphering_op, enum OperationPosition op_position, enum StreamLevel filesize_lvl);
DWORD readTestFile(uint8_t** read_buffer, const WCHAR* file_path, int offset, int length);
DWORD writeTestFile(uint8_t* buffer_to_write, const WCHAR* file_path, int offset, int length);

void getCenteredString(char* str_out, int chars_to_write, const char* str_in);
void printTestTableResults(BOOL use_codes);
void printTestTableLegend();




/////  FUNCTION IMPLEMENTATIONS  /////

void testEverything() {
	testing_mode_on = TRUE;

	if (ERROR_SUCCESS == initTestStreams()) {
		PRINT("INIT TEST STREAMS: OK \n");

		// Iterate through the table doing each uint test
		for (size_t irp_op = 0; irp_op < 2; irp_op++) {
			//irp_op = 1;
			for (size_t op = 0; op < 3; op++) {
				for (size_t op_pos = 0; op_pos < 3; op_pos++) {
					for (size_t fsize_lvl = 0; fsize_lvl < 4; fsize_lvl++) {
						unitTest((enum IrpOperation)irp_op, (enum Operation)op, (enum OperationPosition)op_pos, (enum StreamLevel)fsize_lvl);
					}
				}
			}
		}
		printTestTableResults(FALSE);
		printTestTableLegend();
	}

	//freeTestStreams();		// Do not free to be able to use 6th option
	testing_mode_on = FALSE;
}


DWORD initTestStreams() {
	static initialized = FALSE;
	if (initialized) {
		return ERROR_SUCCESS;
	}

	struct Cipher* p_cipher = NULL;
	struct KeyData* key = NULL;
	uint32_t frn_deciphered = 0;
	uint32_t frn_ciphered = 0;
	int result = 0;

	// Assign cipher
	p_cipher = ctx.folders[0]->protection->cipher;
	if (p_cipher == NULL) {
		return -1;
	}

	// Assign Key
	key = ctx.folders[0]->protection->key;
	result = makeComposedKey(ctx.folders[0]->protection->challenge_groups, key);
	if (0 != result) {
		fprintf(stderr, "ERROR composing key in test (%d)\n", result);
		return -1;
	}

	// Create FRNs
	frn_deciphered = createFRN();
	frn_ciphered = createFRN();

	// Allocate buffer streams. Memory space for the text, but not the last '\0' is allocated.
	// Note: big_cleartext_stream has 1 more allocated byte to contain last '\0'. But it is OK if all the operations are done using big_stream_buffer_size
	// Note 2: following block is a safer equivalent of: big_stream_buffer_size = (DWORD)strlen(test_stream);
	{
		size_t tmp_val = strlen(test_stream);
		if (tmp_val > 0xFFFFFFFF) return -1;
		else big_stream_buffer_size = (DWORD)tmp_val;
	}
	small_stream = malloc(SMALL_STREAM_BUFFER_SIZE * sizeof(uint8_t));
	big_deciphered_stream = malloc(big_stream_buffer_size * sizeof(uint8_t));
	big_cleartext_stream = test_stream;
	big_ciphered_stream = malloc(big_stream_buffer_size * sizeof(uint8_t));
	if (small_stream == NULL || big_deciphered_stream == NULL || big_cleartext_stream == NULL || big_ciphered_stream == NULL) {
		return ERROR_NOT_ENOUGH_MEMORY;
	}
	// Assign streams to 'all_test_streams'
	all_test_streams[SMALL] = small_stream;
	all_test_streams[BIG_DECIPHERED] = big_deciphered_stream;
	all_test_streams[BIG_CLEARTEXT] = big_cleartext_stream;
	all_test_streams[BIG_CIPHERED] = big_ciphered_stream;

	// Create small stream
	memcpy(small_stream, big_cleartext_stream, SMALL_STREAM_BUFFER_SIZE);

	// Create deciphered stream
	invokeDecipher(p_cipher, big_deciphered_stream, big_cleartext_stream, big_stream_buffer_size, 0, key, frn_deciphered);
	mark(big_deciphered_stream, -1, frn_deciphered);

	// Create ciphered stream
	invokeCipher(p_cipher, big_ciphered_stream, big_cleartext_stream, big_stream_buffer_size, 0, key, frn_ciphered);
	mark(big_ciphered_stream, 1, frn_ciphered);

	initialized = TRUE;
	return ERROR_SUCCESS;
}

void freeTestStreams() {
	if (small_stream != NULL) {
		free(small_stream);
		small_stream = NULL;
	}
	if (big_ciphered_stream != NULL) {
		free(big_ciphered_stream);
		big_ciphered_stream = NULL;
	}
	if (big_cleartext_stream != NULL) {
		// Pointer is not freed in this case because it points to a const string
		big_cleartext_stream = NULL;
	}
	if (big_deciphered_stream != NULL) {
		free(big_deciphered_stream);
		big_deciphered_stream = NULL;
	}
	all_test_streams[SMALL] = NULL;
	all_test_streams[BIG_DECIPHERED] = NULL;
	all_test_streams[BIG_CLEARTEXT] = NULL;
	all_test_streams[BIG_CIPHERED] = NULL;

	big_stream_buffer_size = 0;
}


void unitTest(enum IrpOperation irp_op, enum Operation ciphering_op, enum OperationPosition op_position, enum StreamLevel stream_lvl) {
	struct Test* current_test = NULL;
	struct Folder* my_folder = ctx.folders[0];
	struct OpTable* my_op_table = my_folder->protection->op_table;
	WCHAR test_file_path_c[MAX_PATH] = L"";
	WCHAR test_file_path_m[MAX_PATH] = L"";
	WCHAR test_file_name[MAX_PATH] = L"";
	size_t index = 0;
	int offset = 0;
	int length = 0;
	uint8_t pointable_uint8 = 0;
	uint8_t *recyclable_buffer = NULL;
	BOOL aborted = FALSE;

	PRINT("\n=====> STARTING UNIT TEST: irp_op=%d, ciphering_op=%d, op_position=%d, stream_lvl=%d \n", irp_op, ciphering_op, op_position, stream_lvl);


	// Initialize current test
	current_test = &(tests_array[irp_op][ciphering_op][op_position][stream_lvl]);
	current_test->desired_output_stream = NULL;
	current_test->input_stream = NULL;
	current_test->output_stream = NULL;
	current_test->verdict = TEST_VERDICT_NOT_DONE;

	// Check special cases where the test makes no sense itself, and mark them as N/A (which has a success meaning)
	if ((stream_lvl == SMALL && op_position != INSIDE_MARK)
		|| (IRP_OP_WRITE == irp_op && CIPHER == ciphering_op && OUTSIDE_MARK != op_position && BIG_DECIPHERED == stream_lvl)	// cases 1101 and 1111
		|| (IRP_OP_WRITE == irp_op && DECIPHER == ciphering_op && OUTSIDE_MARK != op_position && BIG_CIPHERED == stream_lvl)	// cases 1203 and 1213
		) {
		current_test->verdict = TEST_VERDICT_NOT_APPLICABLE_OK;
		return;
	}

	// Select the input stream
	current_test->input_stream = all_test_streams[stream_lvl];

	// Select offset and length to use in the operation to perform the test
	switch (op_position) {
		case INSIDE_MARK:
			offset = 100;
			length = 200;
			break;
		case INSIDE_AND_OUTSIDE_MARK:
			offset = 450;
			length = 200;
			break;
		case OUTSIDE_MARK:
			offset = 600;
			length = 200;
			break;
	}

	// Create test_file_name
	wcscpy(test_file_name, L"TEST_x_x_x_x.txt");//tmp");
	test_file_name[5] = L'0' + irp_op;
	test_file_name[7] = L'0'+ ciphering_op;
	test_file_name[9] = L'0' + op_position;
	test_file_name[11] = L'0' + stream_lvl;

	// Create file_paths
	//index = wcslen("\\\\?\\_:\\");
	//wcscpy(test_file_path_m, "\\\\?\\_:\\");
	//test_file_path_m[4] = my_folder->mount_point;
	index = wcslen(TEST_M_FOLDER_PATH);
	wcscpy(test_file_path_m, TEST_M_FOLDER_PATH);
	wcscpy(&(test_file_path_m[index]), test_file_name);

	//index = wcslen(my_folder->path);
	//wcscpy(test_file_path_c, my_folder->path);
	index = wcslen(TEST_C_FOLDER_PATH);
	wcscpy(test_file_path_c, TEST_C_FOLDER_PATH);
	wcscpy(&(test_file_path_c[index]), test_file_name);

	// Compute desired output
	switch (ciphering_op) {
		case NOTHING:
			switch (stream_lvl) {
				case SMALL:
					current_test->desired_output_stream = small_stream;
					break;
				case BIG_DECIPHERED:
					current_test->desired_output_stream = big_deciphered_stream;
					break;
				case BIG_CLEARTEXT:
					current_test->desired_output_stream = big_cleartext_stream;
					break;
				case BIG_CIPHERED:
					current_test->desired_output_stream = big_ciphered_stream;
					break;
			}
			break;
		case CIPHER:
			switch (stream_lvl) {
				case SMALL:
					current_test->desired_output_stream = small_stream;
					break;
				case BIG_DECIPHERED:
					current_test->desired_output_stream = big_cleartext_stream;
					break;
				case BIG_CLEARTEXT:
					current_test->desired_output_stream = big_ciphered_stream;
					break;
				case BIG_CIPHERED:
					current_test->desired_output_stream = NULL;			// Note we should check if desired_output_stream is NULL before memcmp()
					break;
			}
			break;
		case DECIPHER:
			switch (stream_lvl) {
				case SMALL:
					current_test->desired_output_stream = small_stream;
					break;
				case BIG_DECIPHERED:
					current_test->desired_output_stream = NULL;			// Note we should check if desired_output_stream is NULL before memcmp()
					break;
				case BIG_CLEARTEXT:
					current_test->desired_output_stream = big_cleartext_stream;
					break;
				case BIG_CIPHERED:
					current_test->desired_output_stream = big_cleartext_stream;
					break;
			}
			break;
	}

	// App type is fixed. Change operative asociated to the operation READ or WRITE
	if (irp_op == IRP_OP_READ) {
		my_op_table->tuples[0]->on_read = ciphering_op;
		//test_operative.on_read = ciphering_op;
	}
	else if (irp_op == IRP_OP_WRITE) {
		my_op_table->tuples[0]->on_write = ciphering_op;
		//test_operative.on_write = ciphering_op;
	}

	// Remove the file and recreate the starting file in monitored drive (through C: for READ  and through M: for WRITE to have it in the mark_table)
	_wremove(test_file_path_c);
	if (irp_op == IRP_OP_READ) {
		writeTestFile(current_test->input_stream, test_file_path_c, 0, (stream_lvl == SMALL) ? SMALL_STREAM_BUFFER_SIZE : big_stream_buffer_size);
	} else if (irp_op == IRP_OP_WRITE) {
		//if (1 == irp_op && 1 == stream_lvl) {
		//	writeTestFile(current_test->input_stream, test_file_path_c, 0, (stream_lvl == SMALL) ? SMALL_STREAM_BUFFER_SIZE : big_stream_buffer_size);
		//} else {
			writeTestFile(current_test->input_stream, test_file_path_m, 0, (stream_lvl == SMALL) ? SMALL_STREAM_BUFFER_SIZE : big_stream_buffer_size);
		//}
	}

	// Execute test using monitored drive and compute output
	if (irp_op == IRP_OP_READ) {
		aborted |= (ERROR_SUCCESS != readTestFile(&(current_test->output_stream), test_file_path_m, offset, length));
	}
	else if (irp_op == IRP_OP_WRITE) {
		aborted |= (ERROR_SUCCESS != writeTestFile(current_test->input_stream, test_file_path_m, offset, length));
		aborted |= (ERROR_SUCCESS != readTestFile(&(current_test->output_stream), test_file_path_c, offset, length));
	}

	// Check the result of the test
	if (current_test->desired_output_stream != NULL) {		// Check the expected result is a correct buffer
		if (current_test->output_stream != NULL) {		// Check the real output is a correct buffer
			// Check output == desired_output
			if (memcmp(&((current_test->desired_output_stream)[offset]), &((current_test->output_stream)[offset]), length) == 0) {
				current_test->verdict = TEST_VERDICT_PASS;
			} else {
				current_test->verdict = TEST_VERDICT_FAIL;
			}
			/*if (ciphering_op == NOTHING) {
				PRINT("\n\n\n\n\n\n\n\n");
				PRINT("memcmp(current_test->desired_output_stream + offset, current_test->output_stream + offset, length): %d\n", memcmp(current_test->desired_output_stream + offset, current_test->output_stream + offset, length));
				PRINT("DESIRED: %*s \n", length, &((current_test->desired_output_stream)[offset]));
				PRINT("OUTPUT : %*s \n", length, &((current_test->output_stream)[offset]));
				PRINT("VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV\n");
				PRINT_HEX(current_test->desired_output_stream + offset, length);
				PRINT_HEX(current_test->output_stream + offset, length);
				PRINT("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
				PRINT("\n\n\n\n\n\n\n\n");
			}*/
		} else {		// The real output is NULL, there was an error
			current_test->verdict = TEST_VERDICT_FAIL;
		}
	} else {		// The expected result is to have aborted
		PRINT("Expecting to have aborted\n");
		if (aborted) {
			PRINT("Aborted as expected: TEST PASSED\n");
			current_test->verdict = TEST_VERDICT_ABORT_OK;
		} else {
			PRINT("Did not abort: TEST FAILED\n");
			current_test->verdict = TEST_VERDICT_FAIL;
		}
	}
}

void unitTestMenu() {
	initTestStreams();

	int n;
	int a, b, c, d = 0;

	printTestTableResults(TRUE);

	printf("Introduce test code: ");
	if (1 != scanf("%d", &n)) return;
	printf("\n");

	// Get indexes
	a = n / 1000;
	b = (n - a * 1000) / 100;
	c = (n - a * 1000 - b * 100) / 10;
	d = (n - a * 1000 - b * 100 - c * 10);

	testing_mode_on = TRUE;
	unitTest(a, b, c, d);
	testing_mode_on = FALSE;

	printTestTableResults(FALSE);
	printTestTableLegend();
}


DWORD readTestFile(uint8_t** read_buffer, const WCHAR* file_path, int offset, int length) {
	FILE* file_ptr = NULL;
	size_t result = 0;
	DWORD error = ERROR_SUCCESS;
	size_t file_size = 0;
	int elem_count = 0;

	//PRINT("read_buffer=%p, file_path=%ws, offset=%d, length=%d\n", read_buffer, file_path, offset, length);

	// Open a file with a wide character path/filename
	result = _wfopen_s(&file_ptr, file_path, L"rb");
	if (result != 0 || file_ptr == NULL) {
		error = ERROR_OPEN_FAILED;
		fprintf(stderr, "ERROR: could not open (read) file (%ws) for the test (error=%lu).\n", file_path, error);
		goto READ_TEST_FILE_CLEANUP;
	}

	// Point to end of file, get the offset (=file size). Point to offset
	fseek(file_ptr, 0L, SEEK_END);
	file_size = ftell(file_ptr);
	fseek(file_ptr, offset, SEEK_SET);

	if (length == -1) {
		elem_count = (int)file_size;
	} else {
		elem_count = length;
	}

	// Allocate enough memory to read the whole file
	*read_buffer = calloc(file_size, sizeof(uint8_t));
	if (*read_buffer == NULL) {
		error = ERROR_NOT_ENOUGH_MEMORY;
		fprintf(stderr, "ERROR: could not allocte memory for the read_buffer in the file (%ws) for the test (error=%lu).\n", file_path, error);
		goto READ_TEST_FILE_CLEANUP;
	}

	// Read from "file_ptr" into "read_buffer"
	// Stops if reaches maximum allocated memory of "read_buffer"
	// Stops if reads "file_size" chunks of "sizeof(uint8_t)" bytes
	result = fread_s(&((*read_buffer)[offset]), file_size-offset, sizeof(uint8_t), elem_count, file_ptr);
	if (result != elem_count || ferror(file_ptr) != 0) {
		error = ERROR_READ_FAULT;
		fprintf(stderr, "ERROR: could not read file (%ws) for the test (error=%lu).\n", file_path, error);
		goto READ_TEST_FILE_CLEANUP;
	}

	// If no errors, set result to success
	fclose(file_ptr);
	return ERROR_SUCCESS;


	// Close file if necessary
	READ_TEST_FILE_CLEANUP:
	if (file_ptr != NULL) {
		fclose(file_ptr);
	}
	/*if (read_buffer != NULL) {					// TO DO uncomment?
		free(read_buffer);
		read_buffer = NULL;
	}*/

	return error;
}

DWORD writeTestFile(uint8_t* buffer_to_write, const WCHAR* file_path, int offset, int length) {
	HANDLE handle = INVALID_HANDLE_VALUE;
	size_t result = 0;
	DWORD error = ERROR_SUCCESS;
	LARGE_INTEGER distanceToMove = { 0 };
	DWORD bytes_written = 0;
	DWORD bytes_to_write = length;

	//PRINT("buffer_to_write=%p, file_path=%ws, offset=%d, length=%d\n", buffer_to_write, file_path, offset, length);
	//PRINT_HEX(buffer_to_write, length);

	// Open a file with a wide character path/filename
	handle = CreateFileW(file_path, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);	// maybe change FILE_ATTRIBUTE_NORMAL with 0
	if (handle == INVALID_HANDLE_VALUE) {
		error = ERROR_READ_FAULT;
		fprintf(stderr, "ERROR: could not open (write) file (%ws) for the test (error=%lu).\n", file_path, error);
		goto WRITE_TEST_FILE_CLEANUP;
	}

	// Point to desired offset
	distanceToMove.QuadPart = offset;
	if (!SetFilePointerEx(handle, distanceToMove, NULL, FILE_BEGIN)) {
		error = GetLastError();
		fprintf(stderr, "ERROR: could not point handle to the desired offset for writting file (%ws) for the test (error=%lu).\n", file_path, error);
		goto WRITE_TEST_FILE_CLEANUP;
	}

	// Write new content to file
	if (!WriteFile(
		handle,
		&(buffer_to_write[offset]),
		bytes_to_write,
		&bytes_written,
		NULL)
		) {
		error = ERROR_WRITE_FAULT;
		fprintf(stderr, "ERROR: could not write file (%ws) for the test.\n", file_path);
		goto WRITE_TEST_FILE_CLEANUP;
	}
	if (bytes_written != length) {
		error = ERROR_WRITE_FAULT;
		fprintf(stderr, "ERROR: could not write file (%ws) for the test.\n", file_path);
		goto WRITE_TEST_FILE_CLEANUP;
	}

	// If no errors, set result to success
	CloseHandle(handle);
	return ERROR_SUCCESS;

	// Close handle if necessary
	WRITE_TEST_FILE_CLEANUP:
	if (handle != INVALID_HANDLE_VALUE) {
		CloseHandle(handle);
	}

	return error;
}


void printTestTableResults(BOOL use_codes) {
	uint32_t max_len_irp_op_str = 0;
	uint32_t max_len_op_str = 0;
	uint32_t max_len_op_pos_str = 0;
	uint32_t max_len_verdict_or_stream_lvl = 0;

	char code[5] = "0000";

	// Reserve space for up to 64 printable characters per column
	char c1[65] = "";
	char c2[65] = "";
	char c3[65] = "";
	char c4[65] = "";
	char c5[65] = "";
	char c6[65] = "";
	char c7[65] = "";

	// A string with 128 underscores (should be more than enough)
	const char underscores[] = "________________________________________________________________________________________________________________________________";

	// Get maximum length of the strings of any column
	// Column 1
	for (size_t i = 0; i < 2; i++) {
		max_len_irp_op_str = MAX(max_len_irp_op_str, (uint32_t)strlen(IRP_OPERATION_STRINGS[i]));
	}
	// Column 2
	for (size_t i = 0; i < 3; i++) {
		max_len_op_str = MAX(max_len_op_str, (uint32_t)strlen(OPERATION_STRINGS[i]));
	}
	// Column 3
	for (size_t i = 0; i < 3; i++) {
		max_len_op_pos_str = MAX(max_len_op_pos_str, (uint32_t)strlen(OPERATION_POSITION_STRINGS[i]));
	}
	// Columns 4, 5, 6, 7 (all together so they have the same width)
	for (size_t i = 0; i < 4; i++) {
		max_len_verdict_or_stream_lvl = MAX(max_len_verdict_or_stream_lvl, (uint32_t)strlen(TEST_VERDICT_STRINGS[i]));
		max_len_verdict_or_stream_lvl = MAX(max_len_verdict_or_stream_lvl, (uint32_t)strlen(FILE_SIZE_AND_LEVEL_STRINGS[i]));
	}


	// The left part (columns c1, c2, c3) of the (6) upper rows is blank (so they are only calculated once)
	getCenteredString(c1, max_len_irp_op_str, "");
	getCenteredString(c2, max_len_op_str, "");
	getCenteredString(c3, max_len_op_pos_str, "");


	getCenteredString(c4, max_len_verdict_or_stream_lvl, underscores);
	getCenteredString(c5, max_len_verdict_or_stream_lvl, underscores);
	getCenteredString(c6, max_len_verdict_or_stream_lvl, underscores);
	getCenteredString(c7, max_len_verdict_or_stream_lvl, underscores);
	printf("  %s   %s   %s  _%s___%s___%s___%s_  \n", c1, c2, c3, c4, c5, c6, c7);

	getCenteredString(c4, max_len_verdict_or_stream_lvl * 4 + 8 + 3 -2, "");
	printf("  %s   %s   %s | %s | \n", c1, c2, c3, c4);

	getCenteredString(c4, max_len_verdict_or_stream_lvl * 4 + 8 + 3 -2, "INPUT STREAM FOR THE WRAPPER");
	printf("  %s   %s   %s | %s | \n", c1, c2, c3, c4);

	getCenteredString(c4, max_len_verdict_or_stream_lvl, underscores);
	getCenteredString(c5, max_len_verdict_or_stream_lvl, underscores);
	getCenteredString(c6, max_len_verdict_or_stream_lvl, underscores);
	getCenteredString(c7, max_len_verdict_or_stream_lvl, underscores);
	printf("  %s   %s   %s |_%s___%s___%s___%s_| \n", c1, c2, c3, c4, c5, c6, c7);

	getCenteredString(c4, max_len_verdict_or_stream_lvl, "");
	getCenteredString(c5, max_len_verdict_or_stream_lvl, "");
	getCenteredString(c6, max_len_verdict_or_stream_lvl, "");
	getCenteredString(c7, max_len_verdict_or_stream_lvl, "");
	printf("  %s   %s   %s | %s | %s | %s | %s | \n", c1, c2, c3, c4, c5, c6, c7);

	getCenteredString(c4, max_len_verdict_or_stream_lvl, FILE_SIZE_AND_LEVEL_STRINGS[0]);
	getCenteredString(c5, max_len_verdict_or_stream_lvl, FILE_SIZE_AND_LEVEL_STRINGS[1]);
	getCenteredString(c6, max_len_verdict_or_stream_lvl, FILE_SIZE_AND_LEVEL_STRINGS[2]);
	getCenteredString(c7, max_len_verdict_or_stream_lvl, FILE_SIZE_AND_LEVEL_STRINGS[3]);
	printf("  %s   %s   %s | %s | %s | %s | %s | \n", c1, c2, c3, c4, c5, c6, c7);


	// From here onwards c1, c2 and c3 are constantly updated (as the rest)
	getCenteredString(c1, max_len_irp_op_str, underscores);
	getCenteredString(c2, max_len_op_str, underscores);
	getCenteredString(c3, max_len_op_pos_str, underscores);
	getCenteredString(c4, max_len_verdict_or_stream_lvl, underscores);
	getCenteredString(c5, max_len_verdict_or_stream_lvl, underscores);
	getCenteredString(c6, max_len_verdict_or_stream_lvl, underscores);
	getCenteredString(c7, max_len_verdict_or_stream_lvl, underscores);
	printf(" _%s___%s___%s_|_%s_|_%s_|_%s_|_%s_| \n", c1, c2, c3, c4, c5, c6, c7);

	getCenteredString(c1, max_len_irp_op_str, "");
	getCenteredString(c2, max_len_op_str, "");
	getCenteredString(c3, max_len_op_pos_str, "");
	getCenteredString(c4, max_len_verdict_or_stream_lvl, "");
	getCenteredString(c5, max_len_verdict_or_stream_lvl, "");
	getCenteredString(c6, max_len_verdict_or_stream_lvl, "");
	getCenteredString(c7, max_len_verdict_or_stream_lvl, "");
	printf("| %s | %s | %s | %s | %s | %s | %s | \n", c1, c2, c3, c4, c5, c6, c7);



	// Iterate over IrpOperation
	for (size_t irp_op = 0; irp_op < 2; irp_op++) {

		// Separator
		if (irp_op != 0) {
			getCenteredString(c1, max_len_irp_op_str, underscores);
			getCenteredString(c2, max_len_op_str, underscores);
			getCenteredString(c3, max_len_op_pos_str, underscores);
			getCenteredString(c4, max_len_verdict_or_stream_lvl, underscores);
			getCenteredString(c5, max_len_verdict_or_stream_lvl, underscores);
			getCenteredString(c6, max_len_verdict_or_stream_lvl, underscores);
			getCenteredString(c7, max_len_verdict_or_stream_lvl, underscores);
			printf("|_%s_|_%s_|_%s_|_%s_|_%s_|_%s_|_%s_| \n", c1, c2, c3, c4, c5, c6, c7);

			getCenteredString(c1, max_len_irp_op_str, "");
			getCenteredString(c2, max_len_op_str, "");
			getCenteredString(c3, max_len_op_pos_str, "");
			getCenteredString(c4, max_len_verdict_or_stream_lvl, "");
			getCenteredString(c5, max_len_verdict_or_stream_lvl, "");
			getCenteredString(c6, max_len_verdict_or_stream_lvl, "");
			getCenteredString(c7, max_len_verdict_or_stream_lvl, "");
			printf("| %s | %s | %s | %s | %s | %s | %s | \n", c1, c2, c3, c4, c5, c6, c7);
		}
		// Iterate over Operation
		for (size_t op = 0; op < 3; op++) {

			// Separator
			if (op != 0) {
				getCenteredString(c1, max_len_irp_op_str, "");
				getCenteredString(c2, max_len_op_str, underscores);
				getCenteredString(c3, max_len_op_pos_str, underscores);
				getCenteredString(c4, max_len_verdict_or_stream_lvl, underscores);
				getCenteredString(c5, max_len_verdict_or_stream_lvl, underscores);
				getCenteredString(c6, max_len_verdict_or_stream_lvl, underscores);
				getCenteredString(c7, max_len_verdict_or_stream_lvl, underscores);
				printf("| %s |_%s_|_%s_|_%s_|_%s_|_%s_|_%s_| \n", c1, c2, c3, c4, c5, c6, c7);

				getCenteredString(c1, max_len_irp_op_str, "");
				getCenteredString(c2, max_len_op_str, "");
				getCenteredString(c3, max_len_op_pos_str, "");
				getCenteredString(c4, max_len_verdict_or_stream_lvl, "");
				getCenteredString(c5, max_len_verdict_or_stream_lvl, "");
				getCenteredString(c6, max_len_verdict_or_stream_lvl, "");
				getCenteredString(c7, max_len_verdict_or_stream_lvl, "");
				printf("| %s | %s | %s | %s | %s | %s | %s | \n", c1, c2, c3, c4, c5, c6, c7);
			}

			// Iterate over OperationPosition and print table results
			if (use_codes) {
				for (size_t op_pos = 0; op_pos < 3; op_pos++) {
					getCenteredString(c1, max_len_irp_op_str, (op_pos == 1 && op == 1) ? IRP_OPERATION_STRINGS[irp_op] : "");
					getCenteredString(c2, max_len_op_str, (op_pos == 1) ? OPERATION_STRINGS[op] : "");
					getCenteredString(c3, max_len_op_pos_str, OPERATION_POSITION_STRINGS[op_pos]);
					sprintf(code, "%04d", irp_op * 1000 + op * 100 + op_pos * 10 + 0);
					getCenteredString(c4, max_len_verdict_or_stream_lvl, code);
					sprintf(code, "%04d", irp_op * 1000 + op * 100 + op_pos * 10 + 1);
					getCenteredString(c5, max_len_verdict_or_stream_lvl, code);
					sprintf(code, "%04d", irp_op * 1000 + op * 100 + op_pos * 10 + 2);
					getCenteredString(c6, max_len_verdict_or_stream_lvl, code);
					sprintf(code, "%04d", irp_op * 1000 + op * 100 + op_pos * 10 + 3);
					getCenteredString(c7, max_len_verdict_or_stream_lvl, code);
					printf("| %s | %s | %s | %s | %s | %s | %s | \n", c1, c2, c3, c4, c5, c6, c7);
				}
			} else {
				for (size_t op_pos = 0; op_pos < 3; op_pos++) {
					getCenteredString(c1, max_len_irp_op_str, (op_pos == 1 && op == 1) ? IRP_OPERATION_STRINGS[irp_op] : "");
					getCenteredString(c2, max_len_op_str, (op_pos == 1) ? OPERATION_STRINGS[op] : "");
					getCenteredString(c3, max_len_op_pos_str, OPERATION_POSITION_STRINGS[op_pos]);
					getCenteredString(c4, max_len_verdict_or_stream_lvl, TEST_VERDICT_STRINGS[tests_array[irp_op][op][op_pos][0].verdict]);
					getCenteredString(c5, max_len_verdict_or_stream_lvl, TEST_VERDICT_STRINGS[tests_array[irp_op][op][op_pos][1].verdict]);
					getCenteredString(c6, max_len_verdict_or_stream_lvl, TEST_VERDICT_STRINGS[tests_array[irp_op][op][op_pos][2].verdict]);
					getCenteredString(c7, max_len_verdict_or_stream_lvl, TEST_VERDICT_STRINGS[tests_array[irp_op][op][op_pos][3].verdict]);
					printf("| %s | %s | %s | %s | %s | %s | %s | \n", c1, c2, c3, c4, c5, c6, c7);
				}
			}
		}
	}

	// Print lower border
	getCenteredString(c1, max_len_irp_op_str, underscores);
	getCenteredString(c2, max_len_op_str, underscores);
	getCenteredString(c3, max_len_op_pos_str, underscores);
	getCenteredString(c4, max_len_verdict_or_stream_lvl, underscores);
	getCenteredString(c5, max_len_verdict_or_stream_lvl, underscores);
	getCenteredString(c6, max_len_verdict_or_stream_lvl, underscores);
	getCenteredString(c7, max_len_verdict_or_stream_lvl, underscores);
	printf("|_%s_|_%s_|_%s_|_%s_|_%s_|_%s_|_%s_| \n", c1, c2, c3, c4, c5, c6, c7);


	/*
	Result should be similar to this:
	                                              ___________________________________________________
												 |                                                   |
												 |           INPUT STREAM FOR THE WRAPPER            |
												 |___________________________________________________|
												 |            |            |            |            |
												 | SMALL  (0) |  BIG (-1)  |  BIG  (0)  |  BIG (+1)  |
	 ____________________________________________|____________|____________|____________|____________|
	|       |          |                         |            |            |            |            |
	|       |          |       INSIDE_MARK       |    PASS    |    PASS    |    PASS    |    PASS    |
	|       |  NOTHING | INSIDE_AND_OUTSIDE_MARK |  N/A (OK)  |    PASS    |    PASS    |    PASS    |
	|       |          |      OUTSIDE_MARK       |  N/A (OK)  |    PASS    |    PASS    |    PASS    |
	|       |__________|_________________________|____________|____________|____________|____________|
	|       |          |                         |            |            |            |            |
	|       |          |       INSIDE_MARK       |    PASS    |    PASS    |    PASS    | ABORT (OK) |
	| READ  |  CIPHER  | INSIDE_AND_OUTSIDE_MARK |  N/A (OK)  |    PASS    |    PASS    | ABORT (OK) |
	|       |          |      OUTSIDE_MARK       |  N/A (OK)  |    PASS    |    PASS    | ABORT (OK) |
	|       |__________|_________________________|____________|____________|____________|____________|
	|       |          |                         |            |            |            |            |
	|       |          |       INSIDE_MARK       |    PASS    | ABORT (OK) |    PASS    |    PASS    |
	|       | DECIPHER | INSIDE_AND_OUTSIDE_MARK |  N/A (OK)  | ABORT (OK) |    PASS    |    PASS    |
	|       |          |      OUTSIDE_MARK       |  N/A (OK)  | ABORT (OK) |    PASS    |    PASS    |
	|_______|__________|_________________________|____________|____________|____________|____________|
	|       |          |                         |            |            |            |            |
	|       |          |       INSIDE_MARK       |    PASS    |    PASS    |    PASS    |    PASS    |
	|       |  NOTHING | INSIDE_AND_OUTSIDE_MARK |  N/A (OK)  |    PASS    |    PASS    |    PASS    |
	|       |          |      OUTSIDE_MARK       |  N/A (OK)  |    PASS    |    PASS    |    PASS    |
	|       |__________|_________________________|____________|____________|____________|____________|
	|       |          |                         |            |            |            |            |
	|       |          |       INSIDE_MARK       |    PASS    |  N/A (OK)  |    PASS    | ABORT (OK) |
	| WRITE |  CIPHER  | INSIDE_AND_OUTSIDE_MARK |  N/A (OK)  |  N/A (OK)  |    PASS    | ABORT (OK) |
	|       |          |      OUTSIDE_MARK       |  N/A (OK)  |    PASS    |    PASS    | ABORT (OK) |
	|       |__________|_________________________|____________|____________|____________|____________|
	|       |          |                         |            |            |            |            |
	|       |          |       INSIDE_MARK       |    PASS    | ABORT (OK) |    PASS    |  N/A (OK)  |
	|       | DECIPHER | INSIDE_AND_OUTSIDE_MARK |  N/A (OK)  | ABORT (OK) |    PASS    |  N/A (OK)  |
	|       |          |      OUTSIDE_MARK       |  N/A (OK)  | ABORT (OK) |    PASS    |    PASS    |
	|_______|__________|_________________________|____________|____________|____________|____________|

	In the syntax of     print("%*.*s", a, b, str);     the vaules of a, b, str represent:
		a = minimum of printed characters adding spaces to the left if necessary.
		b = maximum number of characters of str printed.
		str = the string to be printed (within the conditions set by a, b)

	Print string 'test_str' of lenght 'len' centered in 'space_size' spaces (note:   space_size >= len   for the full test_str to be printed):
	printf(" '%*.*s%*.*s' \n", space_size/2, len/2, test_str, -(space_size-space_size/2), len-len/2, &(test_str[len/2]));


	TEST CODE:

	const char underscores[] = "_________________________________________________";
	const char test_str[] = "holax";
	int len = strlen(test_str);

	printf("Test centrado en 10 con ' ': '%*.*s%*.*s' \n", 5, len/2, test_str, -5, len-len/2, &(test_str[len/2]));
	printf("Test -10.5  con '_': '%*.*s' \n", -10, 5, underscores);
	printf("Test 10.5   con '_': '%*.*s' \n", 10, 5, underscores);
	printf("Test 10.10  con '_': '%*.*s' \n", 10, 10, underscores);
	printf("Test 5.10   con '_': '%*.*s' \n", 5, 10, underscores);
	printf("Test 10.5   con ' ': '%*.*s' \n", 10, 5, "");
	printf("Test 10.10  con ' ': '%*.*s' \n", 10, 10, "");
	printf("Test 5.10   con ' ': '%*.*s' \n", 5, 10, "");
	
	Esto printa:
	Test centrado en 10 con ' ': '   holax  '
	Test -10.5  con '_': '_____     '
	Test 10.5   con '_': '     _____'
	Test 10.10  con '_': '__________'
	Test 5.10   con '_': '__________'

	Test 10.5   con ' ': '          '
	Test 10.10  con ' ': '          '
	Test 5.10   con ' ': '     '

	*/

}


void printTestTableLegend() {
	printf("\n\n");
	printf("These tests are low level and scenarios build up on them. Therefore they allow the validation of any type of scenario which means that the success on these tests ensures the correct functioning of the wrapper independently of the operative table defined by the securemirror user.\n");
	printf("The tests are classified based on the following criteria:\n");
	printf("  * Filesystem operation: IRP_OP_READ (read operation), IRP_OP_WRITE (write operation).\n");
	printf("  * Logic operation: NOTHING (pass the buffer \"as is\"), CIPHER (cipher the buffer), DECIPHER (decipher the buffer).\n");
	printf("  * Part of the buffer where the logical operation is applied: INSIDE_MARK (only affects to the first MARK_LENGTH), OUTSIDE_MARK (only affects to bytes different than the first MARK_LENGTH bytes), INSIDE_AND_OUTSIDE_MARK (affects both).\n");
	printf("  * File size and ciphering level of the wrapper input stream: SMALL (small file, which is always cleartext), BIG_DECIPHERED (deciphered big file), BIG_CLEARTEXT (cleartext big file), BIG_CIPHERED (ciphered big file).\n");
	printf("\n");
	printf("The possible results are:\n");
	printf("  * PASS: test successfully executed and passed.\n");
	printf("  * FAIL: test executed and failed.\n");
	printf("  * ABORT (OK): test executeed and the wrapper aborted the operation, what was expected. Counts as a success value.\n");
	printf("  * N/A (OK): test not executed because it has no valid meaning. Counts as a success value.\n");
	printf("  * NOT DONE: test not executed. This is the default value and should never appear in the tests. Indicates a bug in the testing code and not in the wrapper\n");
	printf("\n\n");
}


// The caller of the function is responsible of ensuring there will be enough space in 'str_out' to hold 'chars_to_write' characters
void getCenteredString(char *str_out, int chars_to_write, const char* str_in) {
	int len = MIN((int)strlen(str_in), chars_to_write);

	sprintf(str_out, "%*.*s%*.*s",
		chars_to_write / 2,							len / 2,			str_in,
		-(chars_to_write - chars_to_write / 2),		len - len / 2,		&(str_in[len / 2])
	);
}

void printUnitTestData() {
	int n;
	int a, b, c, d = 0;
	struct Test* current_test = NULL;
	int length = 0;

	printf("Say number test: ");
	if (1 != scanf("%d", &n)) return;
	printf("\n");

	// Get indexes
	a = n / 1000;
	b = (n - a*1000) / 100;
	c = (n - a*1000 - b*100) / 10;
	d = (n - a*1000 - b*100 - c*10);

	// Set print length
	length = (d == 0) ? 500 : 1000;


	current_test = &(tests_array[a][b][c][d]);
	PRINT("current test %d %d %d %d:\n", a, b, c, d);
	if (current_test->input_stream == NULL) {
		PRINT("INPUT  : NULL \n");
	} else {
		PRINT("INPUT  : %*s \n", length, current_test->input_stream);
	}
	if (current_test->desired_output_stream == NULL) {
		PRINT("DESIRED: NULL \n");
	} else {
		PRINT("DESIRED: %*s \n", length, current_test->desired_output_stream);
	}
	if (current_test->output_stream == NULL) {
		PRINT("OUTPUT : NULL \n");
	} else {
		PRINT("OUTPUT : %*s \n", length, current_test->output_stream);
	}
	PRINT("VERDICT: %d \n", current_test->verdict);

	PRINT_HEX(current_test->input_stream, length);
	PRINT_HEX(current_test->desired_output_stream, length);
	PRINT_HEX(current_test->output_stream, length);
}
