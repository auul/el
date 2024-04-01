#include <el.h>

int main(int argc, char **args)
{
	el_dict *dict = NULL;
	el_data test = Choice(new_choice(Fn(new_built_in(FnUnify, Int(1), Word(new_word("a")), Nil())), Word(new_word("a")), Nil()));

	data_print(test); printf("\n");
	data_print(eval(&dict, test)); printf("\n");
	dict_print(dict); printf("\n");

	return 0;
}
