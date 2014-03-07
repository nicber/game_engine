#pragma once

namespace game_engine
{
	namespace logic
	{
		/** \brief Class responsible for running a game.
		 * Subsystems can be added to it.
		 */
		class game
		{
			std::list<std::unique_ptr<subsystem>> subsystems;

		public:
			/** \brief Adds a new subsystem to the engine.
			 * \param subsys A std::unique_ptr pointing to the subsystem to add.
			 */
			void new_subsystem(std::unique_ptr<subsystem> subsys);

			/** \brief Adds a new subsystem to the engine.
			 * It can directly be constructed directly by the engine for performance.
			 * \param args Paramenters for the constructor of T.
			 */
			template <typename T, typename... Args>
			void new_subsystem(Args&&... args);
		};
	}
}