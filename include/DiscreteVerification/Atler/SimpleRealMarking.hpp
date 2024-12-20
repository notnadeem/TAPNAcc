#ifndef VERIFYTAPN_ATLER_SIMPLEREALMARKING_HPP_
#define VERIFYTAPN_ATLER_SIMPLEREALMARKING_HPP_

#include "DiscreteVerification/Atler/SimpleTimedArcPetriNet.hpp"
#include "SimpleDynamicArray.hpp"
#include "SimpleTimedPlace.hpp"
// #include "SimpleTimedTransition.hpp"
#include <iostream>
#include <limits>

namespace VerifyTAPN::Atler {

struct SimpleRealToken {
  double age;
  int count;

  inline void add(int num) { count = count + num; }
  inline void remove(int num) { count = count - num; }
  inline void deltaAge(double x) { age += x; }
};

struct SimpleRealPlace {
  SimpleTimedPlace place;
  SimpleDynamicArray<SimpleRealToken *> *tokens;

  SimpleRealPlace() { tokens = new SimpleDynamicArray<SimpleRealToken *>(10); }

  SimpleRealPlace(const SimpleRealPlace &other) {
    place = other.place;
    tokens = new SimpleDynamicArray<SimpleRealToken *>(other.tokens->size);
    for (size_t i = 0; i < other.tokens->size; i++) {
      tokens->add(new SimpleRealToken{.age = other.tokens->get(i)->age,
                                      .count = other.tokens->get(i)->count});
    }
  }

  SimpleRealPlace(SimpleTimedPlace place, size_t tokensLength) : place(place) {
    tokens = new SimpleDynamicArray<SimpleRealToken *>(tokensLength);
  }

  ~SimpleRealPlace() {
    for (size_t i = 0; i < tokens->size; i++) {
      delete tokens->get(i);
    }
    delete tokens;
  }

  // TODO: check if this works
  inline void deltaAge(double x) {
    // print all the places
    for (size_t i = 0; i < tokens->size; i++) {
      tokens->get(i)->deltaAge(x);
    }
  }

  inline void addToken(SimpleRealToken &newToken) {
    size_t index = 0;
    for (size_t i = 0; i < tokens->size; i++) {
      SimpleRealToken *token = tokens->get(i);
      if (token->age == newToken.age) {
        token->add(newToken.count);
        return;
      }
      if (token->age > newToken.age)
        break;
      index++;
    }
    // NOTE: Check if this works, might be a issue
    if (index >= tokens->size) {
      tokens->add(&newToken);
    } else {
      tokens->insert(index, &newToken);
    }
  }

  inline double maxTokenAge() const {
    if (tokens->size == 0) {
      return -std::numeric_limits<double>::infinity();
    }
    return tokens->get(tokens->size - 1)->age;
  }

  inline int totalTokenCount() const {
    int count = 0;
    for (size_t i = 0; i < tokens->size; i++) {
      count += tokens->get(i)->count;
    }
    return count;
  }

  double availableDelay() const {
    // TODO: Change to CUDA implementation
    if (tokens->size == 0)
      return std::numeric_limits<double>::infinity();
    double delay = ((double)place.timeInvariant.bound) - maxTokenAge();
    return delay <= 0.0f ? 0.0f : delay;
  }

  inline bool isEmpty() const { return tokens->size == 0; }
};

struct SimpleRealMarking {
  SimpleRealPlace **places;
  size_t placesLength = 0;

  bool deadlocked;

  SimpleRealMarking() {}

  // SimpleRealMarking(const SimpleRealMarking &other) {
  //   placesLength = other.placesLength;
  //   places = other.places;
  //   deadlocked = other.deadlocked;
  // }

  SimpleRealMarking(const SimpleRealMarking &other) {
    placesLength = other.placesLength;
    deadlocked = other.deadlocked;
    places = new SimpleRealPlace*[other.placesLength];
    for (size_t i = 0; i < other.placesLength; i++) {
        places[i] = new SimpleRealPlace(*other.places[i]);
    }
  }

  ~SimpleRealMarking() {
      for (size_t i = 0; i < placesLength; i++) {
        delete places[i];
      }
      delete[] places;
  }

  void deltaAge(double x) {
    for (size_t i = 0; i < placesLength; i++) {
      places[i]->deltaAge(x);
    }
  }

  uint32_t numberOfTokensInPlace(int placeId) const {
    return places[placeId]->totalTokenCount();
  }

  void addTokenInPlace(SimpleTimedPlace &place, SimpleRealToken &newToken) {
    auto token = new SimpleRealToken{.age = newToken.age, .count = newToken.count};
    places[place.index]->addToken(*token);
  }

  bool canDeadlock(const SimpleTimedArcPetriNet &tapn, int maxDelay,
                   bool ignoreCanDelay) const {
    return deadlocked;
  }

  inline bool canDeadlock(const SimpleTimedArcPetriNet &tapn,
                          const int maxDelay) const {
    return canDeadlock(tapn, maxDelay, false);
  };

  double availableDelay() const {
    double available = std::numeric_limits<double>::infinity();
    for (size_t i = 0; i < placesLength; i++) {
      if (places[i]->isEmpty())
        continue;
      double delay = places[i]->availableDelay();
      if (delay < available) {
        available = delay;
      }
    }
    return available;
  }
};

} // namespace VerifyTAPN::Atler

#endif
