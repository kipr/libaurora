#include <string.h>
#include <iostream>

#include <daylite/spinner.hpp>

#include "aurora_frame.hpp"
#include "aurora_key.hpp"
#include "aurora_mouse.hpp"

#include "daylite-node.hpp"

using namespace aurora;
using namespace daylite;
using namespace std;

namespace
{
  template<typename T>
  inline bson_bind::option<T> safe_unbind(const bson_t *raw_msg)
  {
    using namespace bson_bind;
    T ret;
    try
    {
      ret = T::unbind(raw_msg);
    }
    catch(const invalid_argument &e)
    {
      cerr << e.what() << endl;
      return none<T>();
    }

    return some(ret);
  }
}

daylite_node daylite_node::_instance;

daylite_node::daylite_node()
  : _frame_pub(nullptr)
{
}

daylite_node::~daylite_node()
{
  end();
}

bool daylite_node::start()
{
  if(_node)
  {
    cerr << "daylite_node is already initialized" << endl;
    return false;
  }

  if(!(_node = node::create_node("aurora")))
  {
    cerr << "node::create_node failed failed" << endl;
    return false;
  }

  if(!_node->start("127.0.0.1", 8374))
  {
    cerr << "_node->start failed" << endl;
    return false;
  }

  if(!(_frame_pub = _node->advertise("/aurora/frame")))
  {
    cerr << "_node->subscribe('/aurora/frame') failed" << endl;
    return false;
  }

  if(!(_key_events_sub = _node->subscribe("/aurora/key"
    , [](const bson_t *raw_msg, void *usr_arg)
  {
    static_cast<daylite_node*>(usr_arg)->key_events_callback(raw_msg);
  }
  , this)))
  {
    cerr << "_node->subscribe('/aurora/key') failed" << endl;
    return false;
  }

  if(!(_mouse_events_sub = _node->subscribe("/aurora/mouse"
    , [](const bson_t *raw_msg, void *usr_arg)
  {
    static_cast<daylite_node*>(usr_arg)->mouse_events_callback(raw_msg);
  }
  , this)))
  {
    cerr << "_node->subscribe('/aurora/mouse') failed" << endl;
    return false;
  }

  return true;
}

bool daylite_node::spin_once()
{
  return daylite::spinner::spin_once();
}

bool daylite_node::end()
{
  _key_events_sub.reset();
  _mouse_events_sub.reset();
  _frame_pub.reset();
  _node.reset();

  return true;
}

bool daylite_node::publish_frame(const char *format
  , int32_t width
  , int32_t height
  , const vector<uint8_t> &data)
{
  aurora_frame aurora_frame;
  aurora_frame.format = format;
  aurora_frame.width = width;
  aurora_frame.height = height;
  aurora_frame.data = data;

  return _frame_pub->publish(aurora_frame.bind());
}

void daylite_node::key_events_callback(const bson_t *raw_msg)
{
  const auto msg_option = safe_unbind<aurora_key>(raw_msg);

  unordered_set<KeyCode> pressed_keys;

  if(msg_option.some())
  {
    auto msg = msg_option.unwrap();

    for(auto it = msg.key_pressed.begin(); it != msg.key_pressed.end(); ++it)
    {
      pressed_keys.emplace(static_cast<KeyCode>(*it));
    }
  }

  if(_key_events_callback)
  {
    _key_events_callback(pressed_keys);
  }
}

void daylite_node::mouse_events_callback(const bson_t *raw_msg)
{
  const auto msg_option = safe_unbind<aurora_mouse>(raw_msg);

  if(msg_option.none()) return;

  auto msg = msg_option.unwrap();

  if(_mouse_event_callback)
  {
    _mouse_event_callback(
        msg.pos_x.some() ? &msg.pos_x.unwrap() : nullptr
      , msg.pos_y.some() ? &msg.pos_y.unwrap() : nullptr
      , msg.left_button_down.some() ? &msg.left_button_down.unwrap() : nullptr
      , msg.middle_button_down.some() ? &msg.middle_button_down.unwrap() : nullptr
      , msg.right_button_down.some() ? &msg.right_button_down.unwrap() : nullptr);
  }
}
