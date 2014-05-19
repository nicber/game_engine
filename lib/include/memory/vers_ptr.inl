#pragma once

#include <algorithm>
#include <iterator>
#include <stdexcept>

namespace game_engine {
template <typename T, typename Policy>
vers_data<T, Policy>::vers_data(T val)
    : newest_data(this->get_new_time(), std::move(val)) {}

template <typename T, typename Policy>
void vers_data<T, Policy>::remove_older_than(time_point limit) {
  test_invariants();

  if (newest_data.first < limit) {
    throw std::runtime_error("limit passed would remove even the newest value");
  }

  auto it =
      std::lower_bound(old_data.begin(), old_data.end(), limit,
                       [](const time_and_ptr &tim_and_ptr,
                          time_point tim) { return tim_and_ptr.first < tim; });

  old_data.erase(old_data.begin(), it);

  test_invariants();
}

template <typename T, typename Policy>
void vers_data<T, Policy>::remove_newer_than(time_point limit) {
  test_invariants();

  if (limit >= newest_data.first) {
    return;
  }

  if(newest_data.first > limit // we have to remove the newest value
	 && (old_data.size() == 0 || old_data[0].first > limit)) {// and we cant find a suitable replacement.
      throw std::runtime_error(
          "no data available to become the newest_data.value");
	 }

  auto it =
      std::upper_bound(old_data.begin(), old_data.end(), limit,
                       [](time_point tim, const time_and_ptr &tim_and_ptr) {
        return tim < tim_and_ptr.first;
      });

  old_data.erase(it, old_data.end());

  if (newest_data.first > limit) {
    newest_data.first = old_data.back().first;
    newest_data.second = std::move(*old_data.back().second);
    old_data.pop_back();
  }
  test_invariants();
}

template <typename T, typename Policy>
const T &vers_data<T, Policy>::get_latest() const {
  test_invariants();

  return newest_data.second;
}

template <typename T, typename Policy>
const T &vers_data<T, Policy>::get_corresp(time_point time_val) const {
  test_invariants();

  if (newest_data.first == time_val) {
    return newest_data.second;
  }

  auto it = std::find_if(old_data.cbegin(), old_data.cend(),
                         [=](const time_and_ptr &tim_and_ptr) {
    return tim_and_ptr.first == time_val;
  });

  if (it == old_data.cend()) {
    throw std::runtime_error("no data corresponding to passed time point");
  } else {
    return *it;
  }
}

template <typename T, typename Policy>
base_vers_data::time_point
vers_data<T, Policy>::get_time_of(base_vers_data::value_index index) const {
  test_invariants();

  if (index == 0) {
    return newest_data.first;
  }

  index--; // index becomes an index of old_data.

  if (!(index < old_data.size())) {
    throw std::out_of_range("index out of range");
  }

  return (old_data.cbegin() + index)->first;
}

template <typename T, typename Policy>
const T &
vers_data<T, Policy>::get_value_of(base_vers_data::value_index index) const {
  test_invariants();

  if (index == 0) {
    return newest_data.second;
  }

  index--; // index becomes an index of old_data.

  if (!(index < old_data.size())) {
    throw std::out_of_range("index out of range");
  }

  return *((old_data.cbegin() + index)->second);
}

template <typename T, typename Policy>
T &vers_data<T, Policy>::copy_or_get_latest() {
  test_invariants();

  if (this->must_copy(newest_data.first)) {
    old_data.emplace_back(newest_data.first,
                          std::unique_ptr<T>(new T(newest_data.second)));
    newest_data.first = this->get_new_time();
  }

  test_invariants();
  return newest_data.second;
}

template <typename T, typename Policy>
base_vers_data::value_index vers_data<T, Policy>::size() const {
  test_invariants();
  return old_data.size() + 1;
}

template <typename T, typename Policy>
vers_ptr<const vers_data<T, Policy> > vers_data<T, Policy>::ptr() const {
  return vers_ptr<const vers_data<T, Policy> >(*this);
}

template <typename T, typename Policy>
vers_ptr<vers_data<T, Policy> > vers_data<T, Policy>::ptr() {
  return vers_ptr<vers_data<T, Policy> >(*this);
}

template <typename T, typename Policy>
void vers_data<T, Policy>::test_invariants() const {
  if (!std::is_sorted(old_data.cbegin(), old_data.cend())) {
    throw std::logic_error("old_data is not sorted");
  }

  if (old_data.size() > 0 && !(newest_data.first > old_data.back().first)) {
    throw std::logic_error("data stored as newest is not the newest data");
  }
}

template <typename Data> vers_ptr<Data>::vers_ptr(Data &dat) : data(&dat) {}

template <typename Data>
const typename vers_ptr<Data>::value_type &vers_ptr<Data>::operator*() const {
  return data->get_latest();
}

template <typename Data>
typename vers_ptr<Data>::value_type &vers_ptr<Data>::operator*() {
  return data->copy_or_get_latest();
}

template <typename Data>
const typename vers_ptr<Data>::value_type *vers_ptr<Data>::operator->() const {
  return &data->get_latest();
}

template <typename Data>
typename vers_ptr<Data>::value_type *vers_ptr<Data>::operator->() {
  return &data->copy_or_get_latest();
}

template <typename Data>
typename vers_ptr<Data>::value_type *vers_ptr<Data>::get() {
  return &data->copy_or_get_latest();
}

template <typename Data>
const typename vers_ptr<Data>::value_type *vers_ptr<Data>::get() const {
  return &data->get_latest();
}

template <typename Data>
vers_ptr<const Data>::vers_ptr(const Data &dat)
    : data(&dat) {}

template <typename Data>
const typename vers_ptr<const Data>::value_type &vers_ptr<const Data>::
operator*() const {
  return data->get_latest();
}

template <typename Data>
const typename vers_ptr<const Data>::value_type *vers_ptr<const Data>::
operator->() const {
  return &data->get_latest();
}

template <typename Data>
const typename vers_ptr<const Data>::value_type *
vers_ptr<const Data>::get() const {
  return &data->get_latest();
}
}
