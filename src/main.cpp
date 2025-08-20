#include "app.hpp"
#include "logger.hpp"

#include "gap_buffer.hpp"

#include <cstdio>
#include <cstring>

int testGapBufferTiny() {
	GapBuffer gap_buffer;
	if (gap_buffer.loadFile("src/gap_buffer.hpp")) return 1;
	
	// moving left and right
	if (gap_buffer.retreat(10) != 0) return 1;
	if (gap_buffer.advance(10) != 10) return 1;
	if (gap_buffer.retreat(15) != 10) return 1;
	if (gap_buffer.advance(15) != 15) return 1;
	
	size_t temp = gap_buffer.capacity - gap_buffer.post_cursor_index;
	if (gap_buffer.advance(temp) != temp) return 1;
	if (gap_buffer.advance(10) != 0) return 1;
	if (gap_buffer.retreat(10) != 10) return 1;
	if (gap_buffer.advance(15) != 10) return 1;
	if (gap_buffer.retreat(15) != 15) return 1;
	
	printf("moving left and right had no errors\n");
	
	// moving up and down
	gap_buffer.retreat(gap_buffer.pre_cursor_index);
	if (gap_buffer.up(4) != 0) return 1;
	if (gap_buffer.down(4) != 4) return 1;
	if (gap_buffer.up(5) != 4) return 1;
	if (gap_buffer.down(5) != 5) return 1;
	
	gap_buffer.advance(gap_buffer.capacity - gap_buffer.post_cursor_index);
	if (gap_buffer.down(4) != 0) return 1;
	if (gap_buffer.up(4) != 4) return 1;
	if (gap_buffer.down(5) != 4) return 1;
	if (gap_buffer.up(5) != 5) return 1;
	
	printf("moving up and down had no errors\n");
	
	// text manipulation
	if (gap_buffer.up(2) != 2) return 1;
	size_t len = gap_buffer.length();
	gap_buffer.insert("Hello, World!", strlen("Hello, World!"));
	gap_buffer.removeFront(strlen("Hello, World!"));
	if (len != gap_buffer.length()) return 1;
	if (gap_buffer.down(2) != 2) return 1;
	gap_buffer.insert("foobar", strlen("foobar"));
	gap_buffer.up(4);
	
	char lyrics[] = "I'm standin' in the rain\nI'm waiting all alone\nI'm so tired\nI wanna go home";
	if (gap_buffer.insert(lyrics, strlen(lyrics)) != strlen(lyrics)) return 1;
	gap_buffer.up(1);
	
	char text[] = "1234123412341234123412341234231\n";
	for (size_t i = 0; i < 128; i++) {
		gap_buffer.insert(text, strlen(text));
	}
	gap_buffer.up(10);
	len = gap_buffer.length();
	gap_buffer.removeBack(5);
	if (len - 5 != gap_buffer.length()) return 1;
	gap_buffer.end();
	gap_buffer.home();
	gap_buffer.up(1);
	gap_buffer.down(3);
	//gap_buffer.print(stdout);
	
	return 0;
}

int testGapBufferHomeTiny() {
	GapBuffer gap_buffer;
	gap_buffer.loadFile("src/gap_buffer.hpp");
	gap_buffer.end();
	gap_buffer.print(stdout);
	gap_buffer.home();
	gap_buffer.print(stdout);
	
	return 0;
}

int testGapBufferUpDownTiny() {
	GapBuffer gap_buffer;
	gap_buffer.loadFile("test.txt");
	gap_buffer.advance(20);
	gap_buffer.print(stdout);
	gap_buffer.up(1);
	gap_buffer.print(stdout);
	gap_buffer.down(3);
	gap_buffer.print(stdout);
	gap_buffer.down(1);
	gap_buffer.print(stdout);
	
	return 0;
}

int testGapBufferInsertTiny() {
	GapBuffer gap_buffer;
	gap_buffer.loadFile("test.txt");
	if (gap_buffer.insert("Foobar", 6) != 6) return 1;
	gap_buffer.home();
	if (gap_buffer.removeBack(4) != 0) return 1;
	gap_buffer.print(stdout);
	
	gap_buffer.advance(gap_buffer.capacity - gap_buffer.post_cursor_index);
	if (gap_buffer.insert("Foobar", 6) != 6) return 1;
	if (gap_buffer.removeFront(6) != 0) return 1;
	gap_buffer.print(stdout);
	
	return 0;
}

int main(int argc, char **argv) {
	Result result = Logger::init("yadda.log");
	if (result != SUCCESS) {
		return 1;
	}
	
	//if (testGapBufferTiny()) return 1;
	//if (testGapBufferHomeTiny()) return 1;
	// if (testGapBufferUpDownTiny()) return 1;
	// if (testGapBufferInsertTiny()) return 1;
	
	Application app;
	if (argc < 2) {
		result = app.init(nullptr);
	} else {
		result = app.init(argv[1]);
	}
	if (result != SUCCESS) {
		return result;
	}
	app.run();
	
	return 0;
}
// comment
// comment
// comment
// comment
// comment
