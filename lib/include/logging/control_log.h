#include <functional>
#include "logging/log.h"
#include <string>

namespace game_engine {
namespace logging {
enum class policy {
  enable,
  disable
};

/** \brief Sets the fallback policy. It's used when a more specific policy
 * cannot be found.
 * \param pol Policy to set as default.
 */
void set_default_for_all(policy pol);
/** \brief Set a default policy that is scoped to the specified file.
 */
void set_default_for_file(std::string file, policy pol);
/** \brief Sets a policy that applies only to the logging performed by the
 * specified file in the specified line.
 * \param file The file that the logging expression is written in.
 * \param line The line that does the logging.
 * \param pol The policy to apply to the file and line combo.
 */
void set_policy_for_file_line(std::string file, unsigned int line,
                              policy pol);

/** \brief Gets the active policy for the file and line combo.
 * If that combo has not got a dedicated policy it falls back on the file
 * and if it does not exist then it falls back on the default policy.
 * \param file The file to query about.
 * \param line The line to query about.
 * \return The policy that applies to the file and line combo.
 */
policy get_policy_for(const std::string &file, unsigned int line);

/** \brief Gets the file default policy.
 * \param file The file to query about.
 * \return The file default policy.
 */
policy get_policy_for_file(const std::string &file);

/** \brief Gets the default policy for the whole program.
 * \return The global policy.
 */
policy get_default_policy();

/** \brief Removes the file default policy for a specified file.
 * It does not, however, change the file&line policies that apply to this file.
 * \param file File whose file default policy is to be removed.
 */
void remove_file_policy(const std::string &file);
/** \brief Removes the policy that applies to a specific file&line combo.
 * It will rely on file policy and default policy from now on.
 * \param file The file where the logging line is.
 * \param line The line that does the logging.
 */
void remove_file_line_policy(const std::string &file, unsigned int line);

/** \brief Helper struct for the apply_to_all_policies function.
 * The member file indicates the file the policy applies to. If it is an
 * empty string then the policy is the global policy.
 * The line indicates what line of the file the policy applies to. If it is 0
 * then the policy is the file default policy.
 */
struct applied_policy {
  const std::string &file;
  unsigned int line;
  policy pol;

  applied_policy(const std::string &f, unsigned int l, policy p);
};

using visitor_func = std::function<void(applied_policy)>;

/** \brief Applies the passed visitor to all existing policies.
 * It is forbidden to modify policies during the iteration.
 * Any modification decided upon the information from the iteration should be
 * stored somewhere and applied only after finishing.
 * \param vf The visitor to use.
 */
void apply_to_all_policies(visitor_func vf);
}
}
