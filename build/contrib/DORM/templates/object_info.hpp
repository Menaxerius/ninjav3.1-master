#include <string>
#include <vector>

struct Info {
	struct Column {
		std::string								name;
		int										index;
		bool									is_key;
		bool									not_null;
		bool									has_default;
		std::string								cxxtype;
		std::string								conntype;
	};

	struct Navigator {
		std::string								object;
		std::string								function;
		bool									plural;
		std::string								filename;
	};

	std::string								class_name;
	std::string								table_name;
	std::string								basename;

	std::vector<std::string>				keys;
	std::vector<std::vector<std::string>>	indexes;

	std::vector<Column>						columns;
	std::vector<Navigator>					navigators;

	int										autoinc_index;

	std::string								key_params;
};
