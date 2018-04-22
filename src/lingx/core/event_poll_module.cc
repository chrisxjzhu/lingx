#include <lingx/core/event.h>
#include <lingx/core/event_posted.h>
#include <lingx/core/connection.h>
#include <lingx/core/module.h>
#include <lingx/core/conf_file.h>
#include <lingx/core/cycle.h>
#include <lingx/core/process_cycle.h>
#include <lingx/core/times.h>
#include <cassert>
#include <poll.h>

namespace lnx {
namespace {

std::vector<struct pollfd> Events_;
uint nEvents_ = 0;

const char* Poll_init_conf_(Cycle* cycle, MConf* conf);

rc_t Poll_init_(Cycle* cycle, int /*timer*/);
void Poll_done_(Cycle*);
rc_t Poll_add_event_(Event* ev, int event, int flags);
rc_t Poll_del_event_(Event* ev, int event, int flags);
rc_t Poll_process_events_(Cycle* cycle, int timer, int flags);

EventModuleCtx Poll_module_ctx_ {
    "poll",
    nullptr,
    Poll_init_conf_,
    {
        Poll_add_event_,
        Poll_del_event_,
        Poll_add_event_,
        Poll_del_event_,
        nullptr,
        nullptr,
        nullptr,
        Poll_process_events_,
        Poll_init_,
        Poll_done_
    }
};

}

Module Poll_module {
    "lnx_poll_module",
    Poll_module_ctx_,
    {},
    EVENT_MODULE,
    nullptr
};

namespace {

const char* Poll_init_conf_(Cycle* cycle, MConf*)
{
    std::shared_ptr<EventConf> ecf = Get_event_conf(EventConf, cycle, Event_core_module);

    /* It's not me. That's fine. */
    if (ecf->use != Poll_module.ctx_index())
        return CONF_OK;

    return CONF_OK;
}

rc_t Poll_init_(Cycle* cycle, int /*timer*/)
{
    if (Events_.empty())
        nEvents_ = 0;

    if (Process_type >= PROCESS_WORKER
        || cycle->old_cycle() == nullptr
        || cycle->old_cycle()->connection_n() < cycle->connection_n())
    {
        assert(cycle->connection_n() >= Events_.size());

        Events_.resize(cycle->connection_n());
    }

    Io = Os_io;

    Event_actions = Poll_module_ctx_.actions;

    Event_flags = USE_LEVEL_EVENT|USE_FD_EVENT;

    return OK;
}

void Poll_done_(Cycle*)
{
    Events_.clear();
    nEvents_ = 0;
}

rc_t Poll_add_event_(Event* ev, int event, int /*flags*/)
{
    Connection* c = (Connection*) ev->data;

    ev->active = 1;

    if (ev->index != INVALID_INDEX) {
        Log_error(ev->log, Log::ALERT, 0,
                  "poll event fd:%d ev:%d is already set", c->fd, event);
        return OK;
    }

    Log_error(ev->log, Log::DEBUG, 0,
              "poll add event: fd:%d ev:%d", c->fd, event);

    Event* e = (event == POLLIN ? c->write : c->read);

    if (e == nullptr || e->index == INVALID_INDEX) {
        Events_[nEvents_].fd = c->fd;
        Events_[nEvents_].events = (short) event;
        Events_[nEvents_].revents = 0;
        ev->index = nEvents_++;
    } else {
        Log_error(ev->log, Log::DEBUG, 0, "poll add index: %u", e->index);

        Events_[e->index].events |= (short) event;
        ev->index = e->index;
    }

    return OK;
}

rc_t Poll_del_event_(Event* ev, int event, int /*flags*/)
{
    Connection* c = (Connection*) ev->data;

    ev->active = 0;

    if (ev->index == INVALID_INDEX) {
        Log_error(ev->log, Log::ALERT, 0,
                  "poll event fd:%d ev:%d is already deleted", c->fd, event);
        return OK;
    }

    Log_error(ev->log, Log::DEBUG, 0,
              "poll del event: fd:%d ev:%d", c->fd, event);

    Event* e = (event == POLLIN ? c->write : c->read);

    if (e == nullptr || e->index == INVALID_INDEX) {
        --nEvents_;

        if (ev->index < nEvents_) {
            Events_[ev->index] = Events_[nEvents_];

            c = Cur_cycle->files()[Events_[nEvents_].fd];
            if (c->fd == -1) {
                Log_error(ev->log, Log::ALERT, 0, "unexpected last event");
            } else {
                if (c->read->index == nEvents_)
                    c->read->index = ev->index;

                if (c->write->index == nEvents_)
                    c->write->index = ev->index;
            }
        }
    } else {
        Log_error(ev->log, Log::DEBUG, 0, "poll del index: %u", e->index);

        Events_[e->index].events &= (short) ~event;
    }

    ev->index = INVALID_INDEX;

    return OK;
}

rc_t Poll_process_events_(Cycle* cycle, int timer, int flags)
{
    Log_error(cycle->log(), Log::DEBUG, 0, "poll timer: %d", timer);

    int ready = ::poll(Events_.data(), (int) nEvents_, timer);

    int err = (ready == -1) ? errno : 0;

    if (flags & UPDATE_TIME || Event_timer_alarm)
        Time_update();

    Log_error(cycle->log(), Log::DEBUG, 0, "poll ready %d of %u", ready, nEvents_);

    if (err) {
        Log::Level level;

        if (err == EINTR) {
            if (Event_timer_alarm) {
                Event_timer_alarm = 1;
                return OK;
            }

            level = Log::INFO;
        } else {
            level = Log::ALERT;
        }

        Log_error(cycle->log(), level, err, "poll() failed");
        return ERROR;
    }

    if (ready == 0) {
        if (timer != -1)
            return OK;

        Log_error(cycle->log(), Log::ALERT, 0,
                  "poll() returned no events without timeout");
        return ERROR;
    }

    for (uint i = 0; i < nEvents_ && ready; ++i) {
        int revents = Events_[i].revents;

        if (revents & POLLNVAL) {
            Log_error(cycle->log(), Log::ALERT, 0,
                      "poll() error fd:%d ev:%04Xd rev:%04Xd",
                      Events_[i].fd, Events_[i].events, revents);
        }

        if (revents & ~(POLLIN|POLLOUT|POLLERR|POLLHUP|POLLNVAL)) {
            Log_error(cycle->log(), Log::ALERT, 0,
                      "strange poll() events fd:%d ev:%04Xd rev:%04Xd",
                      Events_[i].fd, Events_[i].events, revents);
        }

        if (Events_[i].fd == -1) {
            /*
             * the disabled event, a workaround for our possible bug,
             * see the comment below
             */
            continue;
        }

        Connection* c = Cur_cycle->files()[Events_[i].fd];

        if (c->fd == -1) {
            Log_error(cycle->log(), Log::ALERT, 0, "unexpected event");

            /*
             * it is certainly our fault and it should be investigated,
             * in the meantime we disable this event to avoid a CPU spinning
             */
            if (i == nEvents_ - 1)
                --nEvents_;
            else
                Events_[i].fd = -1;

            continue;
        }

        if (revents & (POLLERR|POLLHUP|POLLNVAL)) {
            /*
             * if the error events were returned, add POLLIN and POLLOUT
             * to handle the events at least in one active handler
             */
            revents |= POLLIN|POLLOUT;
        }

        bool found = false;

        if ((revents & POLLIN) && c->read->active) {
            found = true;

            Event* ev = c->read;
            ev->ready = 1;

            Queue* queue = ev->accept ? &Posted_accept_events : &Posted_events;

            Post_event(ev, queue);
        }

        if ((revents & POLLOUT) && c->write->active) {
            found = true;

            Event* ev = c->write;
            ev->ready = 1;

            Post_event(ev, &Posted_events);
        }

        if (found) {
            --ready;
            continue;
        }
    }

    if (ready != 0)
        Log_error(cycle->log(), Log::ALERT, 0, "poll ready != events");

    return OK;
}

}

}
