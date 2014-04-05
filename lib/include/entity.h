#pragma once

#include "component.h"

#include <memory>
#include <typeindex>
#include <unordered_map>

namespace game_engine
{
	namespace logic
	{
		class game;

		/** \brief Basic class that is the base class of all entities. */
		class entity
		{
			friend class game;
			friend class component;
			/** \brief Stores the components added to this entity. */
			std::unordered_map<std::type_index, std::unique_ptr<component>> components;

			/** \brief Stores a pointer to the game that owns this entity. */
			game* parent_game = nullptr;
		protected:
			/** \brief Adds a new component to the entity. */
			template <typename T, typename... Args>
			void new_component(Args&&... args);

			/** \brief Adds an already constructed component to the entity. */
			void add_component(std::unique_ptr<component> comp);

			/** \brief Removes a component from the entity, and, through
			 * ~component() from any subsystem that had accepted it.
			 * Throws component_not_found<T> if no component of type T
			 * was part of this entity.
			 */
			template <typename T>
			void remove_component();

			/** \brief Tries to remove a component of type T. If it couldn't
			 * be removed then it returns false. Otherwise it returns true.
			 */
			template <typename T>
			bool try_remove_component();

			/** \brief Returns a pointer to the component of type T part 
			 * of this entity. If there isn't any, then it returns nullptr.
			 */
			template <typename T>
			T* try_get_comp() const;

			/** \brief Returns a reference to the component of type T part
			 * of this entity. Otherwise it throws component_not_found<T>.
			 */
			template <typename T>
			T& get_comp() const;

			/** \brief Returns the game that owns the this entity. */
			game& get_parent_game() const;
		public:
			virtual ~entity();
			
			template <typename T>
			class component_not_found : std::runtime_error
			{
			public:
				component_not_found(std::string mess = "") : std::runtime_error(std::move(mess))
				{}
			};

			class component_exists : public std::runtime_error
			{
			public:
				component_exists(std::string mess = "") : std::runtime_error(std::move(mess))
				{}
			};
		};
	}
}

#include "entity.inl"