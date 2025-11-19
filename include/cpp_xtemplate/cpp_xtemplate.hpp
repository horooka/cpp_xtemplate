#include <gtkmm.h>
#include <glibmm.h>

class TemplateCols : public Gtk::TreeModel::ColumnRecord {
    public:
        TemplateCols() {
            add(name);
            add(is_hardcoded);
            add(vars_types);
            add(vars_names);
            add(tags);
            add(body);
        }

    Gtk::TreeModelColumn<Glib::ustring> name;
    Gtk::TreeModelColumn<bool> is_hardcoded;
    Gtk::TreeModelColumn<std::vector<std::string>> vars_types;
    Gtk::TreeModelColumn<std::vector<std::string>> vars_names;
    Gtk::TreeModelColumn<std::vector<std::string>> tags;
    Gtk::TreeModelColumn<Glib::ustring> body;
};

const int STATE_SEARCH = 0;
const int STATE_TEMPLATE_OPENED = 1;
const int STATE_TEMPLATE_CREATION = 2;

std::string trim(const std::string &s);
std::vector<std::string> split_by_comma(const std::string &s);
void set_margin(Gtk::Widget& widget, int margin_horizontal, int margin_vertical);
int parse_config_hardcoded(const std::string &content, Glib::RefPtr<Gtk::ListStore> liststore_tempalates, const TemplateCols &template_cols);
int parse_config(const std::string &config_path, Glib::RefPtr<Gtk::ListStore> liststore_templates, const TemplateCols& template_cols);
int write_config(const std::string &config_path, const Glib::RefPtr<Gtk::ListStore> &liststore_templates, const TemplateCols& template_cols);
