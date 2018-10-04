#ifndef test01split_h
#define test01split_h
#endif /* test01split_h */

#include <tuple>
#include <utility>
#include <iostream>
#include <functional>
#include <istream>
#include <sstream>
#include <stdexcept>
#include <type_traits>

// Ondřej Měkota
// 7. 4. 2018

namespace split_internal {
    //does things only if the last item is lvalue
    template<typename C, typename P, typename TStream, bool readB = false>
    struct reading{
        template<size_t Index = 0>
        static bool read(C && current, P && previous, TStream && stream){
            if(readB){
                return reading<char, C, TStream>::read(std::forward<char>('\n'), std::forward<C>(current), std::forward<TStream>(stream));
            }
            return true;
        }
    };
    
    // lvalue reference then char in tuple
    template<typename P, typename TStream>
    struct reading<char, P, TStream, false>{
        template<size_t Index = 0>
        static bool read(char && current, P && previous, TStream && stream){
            std::string sink;
            std::string sinkBak = "";
            std::getline(stream, sink, current);
            std::istringstream iss(sink);
            iss >> previous;
            
            if (iss.fail() && sink != ""){
                
                // different line endings - \r\n
                //-----
                // The assignment says to use '\n' when no other delimiter is provided but the test files contain '\r\n' on some (about 9000) lines,
                // so in order to obtain correct (provided) results, this fix is nessesary.
                if(current == '\n' && sink == "\r")
                    return true;
                return false;
            }
            else if (!iss.eof()){
                std::string s2;
                iss >> s2;
                if(s2 != "")
                    return false;
            }
            return true;
        }
    };
    
    // char then char in tuple
    template<typename TStream>
    struct reading<char, char, TStream, false>{
        template<size_t Index = 0>
        static bool read(char && current, char && previous, TStream && stream){
            char c;
            stream >> c;
            if(c != current || stream.eof())
                throw std::logic_error("Wrong delimiter or EOF encountered.");
            return true;
        }
    };
    
    //called if pack of args starts with lvalue reference -> "do nothing and 'wait' for next arg (which is delimiter)"
    template<typename C, typename TStream>
    struct reading2{
        template<size_t Index = 0>
        static bool read(C && current, TStream && stream){
            return true;
        }
    };
    
    //called if pack of args starts with char -> "throw away everything before (and including) delimiter"
    template<typename TStream>
    struct reading2<char, TStream>{
        template<size_t Index = 0>
        static bool read(char && current, TStream && stream){
            std::string dump;
            std::getline(stream, dump, current);
            return true;
        }
    };
    
    //iterates through tuple
    template<typename TTuple, typename TStream>
    struct iterate_tuple{
        template<size_t Index = 0, size_t Size = std::tuple_size_v<std::remove_reference_t<TTuple>>>
        static void doit(TTuple && tuple, TStream && stream){
            typedef decltype(std::tuple_element_t<Index, TTuple>(std::get<Index>(tuple))) currentType;
            
            if constexpr (Index > 0){
                constexpr size_t previousIndex = Index - 1;
                typedef decltype(std::tuple_element_t<previousIndex, TTuple>(std::get<previousIndex>(tuple))) previousType; //type if previous argument on split
                constexpr bool read = Index + 1 == Size && std::is_lvalue_reference<currentType>() ? true : false; //if the last argument is lvalue reference, stream is read until '\n'
                auto result = reading<currentType,previousType, TStream, read>::
                    read(
                         std::forward<currentType>(std::get<Index>(tuple)),
                         std::forward<previousType>(std::get<previousIndex>(tuple)),
                         std::forward<TStream>(stream)
                    );
                if(!result)
                    throw std::logic_error("Delimiter has not been found or operator >> did not work correctly.");
            }
            else{
                //first argument
                reading2<currentType, TStream>::
                    read(
                         std::forward<currentType>(std::get<Index>(tuple)),
                         std::forward<TStream>(stream)
                    );
            }
            
            if (stream.fail() || stream.bad()){
                throw std::logic_error("Operator >> did not work correctly.");
            }
            if constexpr (Index + 1 < Size)
                doit<Index + 1>(std::forward<TTuple>(tuple), std::forward<TStream>(stream));
        }
    };

    //this checks whether there are no adjacent lvalue references nor non char rvalue on the input.
    //-------------
    //However STD::STIRNG literal is LVALUE REFERENCE (!) so this program considers it as one (and it will most likely fail if such argument provided).
    //It should not be hard to check if argument is lvalue reference and string literal but it was not specified in the assignment so I did not implement it.
    inline void check(){};
    
    template<typename First>
    inline void check(First&& first){
        static_assert(std::is_same<First, char>() || std::is_lvalue_reference<decltype(std::forward<First>(first))>(), "Argument is neither char nor lvalue reference.");
    };
    
    template<typename First, typename ... Rest>
    inline void check(First&& first, Rest&& ... rest){
        static_assert(std::is_same<First, char>() || std::is_lvalue_reference<decltype(std::forward<First>(first))>(), "Argument is neither char nor lvalue reference.");
        check(std::forward<Rest>(rest)...);
    };
    
    template<typename First, typename Second, typename... Rest>
    inline void check(First && first, Second&& second, Rest && ... rest){
        static_assert(
                      std::is_same<First, char>() ||
                      std::is_lvalue_reference<decltype(std::forward<First>(first))>(),
                  "Argument is neither char nor lvalue reference."
        );
        static_assert(
                      !(std::is_lvalue_reference<decltype(std::forward<First>(first))>() &&
                        std::is_lvalue_reference<decltype(std::forward<Second>(second))>()),
                  "Two consecutive lvalue references."
        );
        check(std::forward<Second>(second), std::forward<Rest>(rest) ...);
    };
};


//main (public) namespace
namespace splitter {
    using namespace split_internal;
    
    //this encloses arguments of function split
    template<typename ... T>
    struct pack{
        pack(std::tuple<T...> && input): data{input} { };
        std::tuple<T...> data;
    };
    
    template<typename ... T>
    inline std::istream & operator>>(std::istream & is, pack<T...> p){
        iterate_tuple<decltype(p.data), std::istream>::
            doit(
                std::forward<decltype(p.data)>(p.data),
                std::forward<std::istream>(is)
            );
        return is;
    };
    
    //split function
    template<typename ... T>
    inline pack<T...> split(T && ... input){
        split_internal::check(std::forward<T>(input) ...);
        pack<T...> result(std::forward_as_tuple(input...));
        return result;
    };
};
