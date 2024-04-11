

#ifndef CARBON_EXCEPTION_SPECIFICATION_HPP
#define CARBON_EXCEPTION_SPECIFICATION_HPP

#include <memory>

#include "../defines.hpp"
#include "boxed_cast.hpp"
#include "dispatchkit.hpp"

namespace Carbon {
namespace detail {
struct Exception_Handler_Base {
    virtual void handle(const Boxed_Value &bv,
                        const Dispatch_Engine &t_engine) = 0;

    virtual ~Exception_Handler_Base() = default;

protected:
    template <typename T>
    static void throw_type(const Boxed_Value &bv,
                           const Dispatch_Engine &t_engine) {
        try {
            T t = t_engine.boxed_cast<T>(bv);
            throw t;
        } catch (const Carbon::exception::bad_boxed_cast &) {
        }
    }
};

template <typename... T>
struct Exception_Handler_Impl : Exception_Handler_Base {
    void handle(const Boxed_Value &bv,
                const Dispatch_Engine &t_engine) override {
        (throw_type<T>(bv, t_engine), ...);
    }
};
}  // namespace detail

/// \brief Used in the automatic unboxing of exceptions thrown during script
/// evaluation
///
/// Exception specifications allow the user to tell ChaiScript what possible
/// exceptions are expected from the script being executed. Exception_Handler
/// objects are created with the Carbon::exception_specification() function.
///
/// Example:
/// \code
/// Carbon::ChaiScript chai;
///
/// try {
///   chai.eval("throw(runtime_error(\"error\"))",
///   Carbon::exception_specification<int, double, float, const std::string &,
///   const std::exception &>());
/// } catch (const double e) {
/// } catch (int) {
/// } catch (float) {
/// } catch (const std::string &) {
/// } catch (const std::exception &e) {
///   // This is the one what will be called in the specific throw() above
/// }
/// \endcode
///
/// It is recommended that if catching the generic \c std::exception& type that
/// you specifically catch the Carbon::exception::eval_error type, so that there
/// is no confusion.
///
/// \code
/// try {
///   chai.eval("throw(runtime_error(\"error\"))",
///   Carbon::exception_specification<const std::exception &>());
/// } catch (const Carbon::exception::eval_error &) {
///   // Error in script parsing / execution
/// } catch (const std::exception &e) {
///   // Error explicitly thrown from script
/// }
/// \endcode
///
/// Similarly, if you are using the Carbon::eval form that unboxes the return
/// value, then Carbon::exception::bad_boxed_cast should be handled as well.
///
/// \code
/// try {
///   chai.eval<int>("1.0", Carbon::exception_specification<const std::exception
///   &>());
/// } catch (const Carbon::exception::eval_error &) {
///   // Error in script parsing / execution
/// } catch (const Carbon::exception::bad_boxed_cast &) {
///   // Error unboxing return value
/// } catch (const std::exception &e) {
///   // Error explicitly thrown from script
/// }
/// \endcode
///
/// \sa Carbon::exception_specification for creation of
/// Carbon::Exception_Handler objects \sa \ref exceptions
using Exception_Handler = std::shared_ptr<detail::Exception_Handler_Base>;

/// \brief creates a Carbon::Exception_Handler which handles one type of
/// exception unboxing \sa \ref exceptions
template <typename... T>
Exception_Handler exception_specification() {
    return std::make_shared<detail::Exception_Handler_Impl<T...>>();
}
}  // namespace Carbon

#endif
