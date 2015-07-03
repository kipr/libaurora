#ifndef _AURORA_HPP_
#define _AURORA_HPP_

#include <functional>
#include <memory>
#include <unordered_set>
#include <vector>

#include <bson.h>
#include <daylite/node.hpp>
#include <daylite/publisher.hpp>
#include <daylite/subscriber.hpp>

#include "kipr/graphics-key-code.h"

namespace aurora
{
  class daylite_node
  {
  public:
    typedef std::function < void(const int32_t *pos_x
      , const int32_t *pos_y
      , const bool *left_down
      , const bool *middle_down
      , const bool *right_down) > mouse_event_callback_t;
    typedef std::function<void(const std::unordered_set<KeyCode> &pressed_keys)> key_events_callback_t;

    static daylite_node &ref() { return _instance; }

    daylite_node(const daylite_node &) = delete;
    daylite_node &operator=(daylite_node const &) = delete;

    bool start();
    bool spin_once();
    bool end();

    void set_key_events_callback(key_events_callback_t cb) { _key_events_callback = cb; }
    void set_mouse_event_callback(mouse_event_callback_t cb) { _mouse_event_callback = cb; }

    bool publish_frame(const char *format
      , int32_t width
      , int32_t height
      , const std::vector<uint8_t>& data);

  private:
    daylite_node();
    ~daylite_node();

    static daylite_node _instance;

    void key_events_callback(const bson_t *msg);
    void mouse_events_callback(const bson_t *msg);

    std::shared_ptr<daylite::node> _node;
    std::shared_ptr<daylite::publisher> _frame_pub;
    std::shared_ptr<daylite::subscriber> _key_events_sub;
    std::shared_ptr<daylite::subscriber> _mouse_events_sub;

    key_events_callback_t _key_events_callback;
    mouse_event_callback_t _mouse_event_callback;
  };
}

#endif
