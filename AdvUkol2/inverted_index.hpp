
#ifndef _ii_hpp
#define _ii_hpp

#include <iostream>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <vector>
#include <memory>
#include <atomic>
#include <future>
#include <chrono>
#include <utility>
#include <limits>


namespace ii_internal {
    
    enum ThreadState {
        ready, running, finished
    };

    enum int_type {int8, int16, int32, int64};

    
    
    typedef std::vector<uint64_t> Objects;
    
    typedef struct FeatureOnDiskType{
        typedef uint64_t* iterator;
        typedef const uint64_t* const_iterator;
        typedef uint64_t const_reference;
    };
    
    template<typename T, typename S>
    struct FeatureData{
        FeatureData(){}
    };
    
    
    template<typename S>
    struct FeatureData<FeatureOnDiskType, S> {
        typedef S* iterator;
        typedef const S* const_iterator;
        typedef S const_reference;

        FeatureData(const uint64_t * i, uint64_t f){
            if(f >= *i)
                std::out_of_range("Feature out of range");
            
            //zacatek oblasti s XB, 1..0, 2..1, 4..2, 8..3:    i + 1 + *(i + 1 + 10*f + 2 + K*2 + 1)
            //pocet prvku oblasti s XB, 1..0, 2..1, 4..2, 8..3:    i + 1 + *(i + 1 + 10*f + 2 + K*2)
            //konec oblasti (krome 8) s XB, 1..0, 2..1, 4..2, 8..3:    i + 1 + *(i + 1 + 10*f + 2 + (K+1)*2 + 1)
            
            if constexpr (sizeof(S) == 1){
                start = (const_iterator) (i + 1 + *(i + 1 + 10*f + 2 + 1));
                end2 = (const_iterator) (start + *(i + 1 + 10*f + 2));
            }
            else if constexpr (sizeof(S) == 2){
                start = (const_iterator) (i + 1 + *(i + 1 + 10*f + 2 + 2 + 1));
                end2 = (const_iterator) (start + *(i + 1 + 10*f + 2 + 2));

            }
            else if constexpr (sizeof(S) == 4){
                start = (S*) (i + 1 + *(i + 1 + 10*f + 2 + 4 + 1));
                end2 = (const_iterator) (start + *(i + 1 + 10*f + 4 + 2));

            }
            else {
                start = (const_iterator) (i + 1 + *(i + 1 + 10*f + 2 + 6 + 1));
                end2 = (const_iterator) (i + 1 + *(i + 1 + 10*(f+1) + 2 + 1)); //zacatek dalsi

            }
        };
        
        const_iterator begin() const noexcept {
            return start;
        }
        const_iterator end() const noexcept {
            return end2;
        }
        const_iterator& operator++() {
            return ++ptr;
        }
        bool operator!=(const iterator& other) const {
            return other != ptr;
        }
        bool operator!=(const const_iterator& other) const {
            return other != ptr;
        }
        bool operator==(const iterator& other) const {
            return other == ptr;
        }
        bool operator==(const const_iterator& other) const {
            return other == ptr;
        }
        const_reference operator*() const {
            return *ptr;
        }
        uint64_t size() const {
            return 999999;
        }
    private:
        const_iterator start;
        const_iterator end2;
        const_iterator ptr;
    };
    
    template<typename T>
    struct FeatureDataItem{
        FeatureDataItem(const uint64_t * i, uint64_t f, int_type s){
            if(s == int8){
                chosen = int8;
                d1 = FeatureData<T, uint8_t>(i, f);
            }
            else if(s == int16){
                chosen = int16;
                d2 = FeatureData<T, uint16_t>(i, f);
            }
            else if(s == int32){
                chosen = int32;
                d4 = FeatureData<T, uint32_t>(i, f);
            }
            else {
                chosen = int64;
                d8 = FeatureData<T, uint64_t>(i, f);
            }
        }
        
        
        int_type chosen;
        
        union {
            FeatureData<T, uint8_t> d1;
            FeatureData<T, uint16_t> d2;
            FeatureData<T, uint32_t> d4;
            FeatureData<T, uint64_t> d8;
        };
        union {
            const uint64_t * const_iterator1;
            const uint64_t * const_iterator2;
            const uint64_t * const_iterator4;
            const uint64_t * const_iterator8;
        };
        union {
            const uint64_t const_reference1;
            const uint64_t const_reference2;
            const uint64_t const_reference4;
            const uint64_t const_reference8;
        };


    };
    template<typename T>
    struct FeatureDataEncloser{
    };

                            
    template<>
    struct FeatureDataEncloser<FeatureOnDiskType>{
        FeatureDataEncloser(const uint64_t * i, uint64_t f){

        }
        std::vector<FeatureDataItem<FeatureOnDiskType>> fdi;
    };
    
    
    template<typename T>
    struct FeatureEncloser{};
    
    template<>
    struct FeatureEncloser<Objects>{
        FeatureEncloser(): data{Objects()}, id{1}{}
        FeatureEncloser(uint64_t f): data{Objects()}, id{f}{}
        uint64_t getId() const {
            return id;
        }
        uint64_t & getId() {
            return id;
        }

        Objects data;
    private:
        uint64_t id;

    };
    
    template<>
    struct FeatureEncloser<FeatureOnDiskType>{
        FeatureEncloser(const uint64_t * i, uint64_t f): id{f} {
            data.push_back(FeatureDataItem<FeatureOnDiskType>(i, f, int8));
            data.push_back(FeatureDataItem<FeatureOnDiskType>(i, f, int16));
            data.push_back(FeatureDataItem<FeatureOnDiskType>(i, f, int32));
            data.push_back(FeatureDataItem<FeatureOnDiskType>(i, f, int64));

        }
        uint64_t getId() const {
            return id;
        }

        std::vector<FeatureDataItem<FeatureOnDiskType>> data;
    private:
        uint64_t id;

    };

    //TODO delete
    template<typename S>
    inline void print(S s){
        std::cerr << "" << s <<  " ";
    }
    
    template<typename T>
    struct FeatureState{
//        FeatureState<T>(FeatureEncloser<T> && fe): state{fe.begin()}, feature{fe}{};
//        typename T::const_iterator state;
//        FeatureEncloser<T> & feature;
        FeatureEncloser<Objects> & feature;
    };
    
    template<>
    struct FeatureState<FeatureOnDiskType>{
        FeatureState(FeatureEncloser<FeatureOnDiskType> && fe): feature{fe}{
        };
        FeatureEncloser<FeatureOnDiskType> & feature;
    };

    
    template<typename T>
    inline void searchThead(FeatureEncloser<Objects> & documents, std::vector<FeatureState<T>> & stateForFeature, std::atomic<ThreadState> & state){
        bool again = true;
        uint64_t max = 0;
        bool found = false;
        
        uint64_t lastFeatureId = stateForFeature.front().feature.getId();
        bool first = true;
        do {
            for(auto && fs:stateForFeature){
                auto a = fs.feature;
                for(auto && dt:fs.feature.data){
                    switch (dt.chosen) {
                        case int8:{
                            auto start = dt.const_iterator1;
                            auto ds = dt.d1;
                            break;
                        }
                        case int16:{
                            auto start = dt.const_iterator2;
                            auto ds = dt.d2;

                            break;
                        }
                        case int32:{
                            auto start = dt.const_iterator4;
                            auto ds = dt.d4;

                            break;
                        }
                        default:{
                            auto start = dt.const_iterator8;
                            auto ds = dt.d8;

                            break;
                        }
                    }
                    for(auto it = start; it !=  ds.end(); ++it){
                    
                    }
                }
            }
        } while(true);
//                for (auto it = fs.state; it != fs.feature.data.end(); ++it){
//                    fs.state = it;
//
//                    if(it == fs.feature.data.end() - 1){
//                        if(*it < max){
//                            state= finished;
//                            return;
//                        }
//                    }
//                    if(*it > max){
//                        max = *it;
//                        lastFeatureId = fs.feature.getId();
//                        first = false;
//                        break;
//                    }
//                    else if(*it == max){
//                        if(lastFeatureId == fs.feature.getId() && !first){
//                            found = true;
//                            first = false;
//                        }
//                        if(first){
//                            lastFeatureId = fs.feature.getId();
//                            first = false;
//                        }
//                        if(found && it == fs.feature.data.end() - 1){
//                            again = false;
//                        }
//                        break;
//                    }
//                }
//                if(found)
//                    break;
//            }
//            if(found){
//                documents.data.push_back(max);
//                found = false;
//                for(auto && fs:stateForFeature)
//                    fs.state++;
//            }
        } while(again);
        state=finished;
        return;
        
    }
    

    //One thread of features
    template<typename T>
    struct FeatureBunch{
        
        void addFeature(FeatureEncloser<T> && f){
            FeatureState<T> fs(std::forward<decltype(f)>(f));
            stateForFeature.push_back(fs);
        }
        
        std::vector<FeatureState<T>> stateForFeature; //featury nemaji po sobe jsouci cisla, na i-te pozici bude i-ta featura
    };
    
    struct ThreadStateLayer {
        ThreadStateLayer(){};
//        ThreadStateLayer(uint64_t t, uint64_t l): threadId{t}, state{ready}, layer{l}{};
        uint64_t threadId;
        std::atomic<ThreadState> state;
        uint64_t layer;
    };
    
    struct LayerInfo{
        LayerInfo():awaiting{0}, first{true}, odd{false}, size{0}{};
        bool first, odd;
        uint64_t awaiting;
        uint64_t size;
        std::vector<FeatureEncloser<Objects>> data;
        std::vector<uint64_t> queue; //id v "data" ktera uz jsou dokoncena
    };
    
    template<size_t threads, typename T>
    struct Finder{
        Finder(std::vector<FeatureEncloser<T>> f): features2{f}, featuresCount{f.size()}{
            documents = std::vector<FeatureEncloser<Objects>>();
        }

        const FeatureEncloser<Objects> & search(){
            
            splitToThreads();
            std::vector<std::thread> threadPool;
            ThreadStateLayer threadInfo[threads];
            std::vector<LayerInfo> layers;
            uint64_t freeThreads = featureBunches.size();
            std::atomic<uint64_t> currentCount = 0; //aktualni pocet "mezivysledku"
            
            uint64_t layersNr = log2(featureBunches.size()) + 1;
            
            layers.push_back(LayerInfo());
            for(uint64_t l = 1; l < layersNr; ++l){
                layers.push_back(LayerInfo());
                if(l != 1){
                    layers[l].data.resize(featureBunches.size() >> (2 * (l-1)));
                    layers[l].odd = featureBunches.size() % (2 * (l-2)) == 1; //TODO je toto spravne?
                }
                else{
                    layers[l].data.resize(featureBunches.size());
                    layers[l].odd = featureBunches.size() % 2 == 1;
                }

            }

            
            //First: iteration through feature on disk
            for(size_t i  = 0; i < featureBunches.size(); ++i){
                threadInfo[i].threadId = i;
                threadInfo[i].state = running;
                threadInfo[i].layer = 0;
                --freeThreads;
                
                layers[0].size++;

                //stores documents in layer[1].
                threadPool.push_back(std::thread(searchThead<T>, std::ref(layers[1].data[i]), std::ref(featureBunches[i].stateForFeature), std::ref(threadInfo[i].state)));
                currentCount++;
            }
            uint64_t layer = 0; // TODO dodelat

            //Second: iteration through the result of "First"
            while(true){
                for(uint64_t i = 0; i < threadPool.size(); ++i){
                    uint64_t cl = threadInfo[i].layer;
                    if(cl >= layersNr - 2){ //predposledni nebo posledni
                        if(threadInfo[i].state == finished && cl == layersNr - 2){ //musi skoncit vsechny ( == prave 1) thready na predposledni vrstve
                            threadPool[i].join();
                            return layers[layersNr - 1].data[0];
                            
                            //TODO !!!!! tohle nefunguje... nejak prepocitat vrstvy.
                        }
                        continue;
                    }
                    else{ //pro vsechny vrstvy krome posledni a predposledni
                        
                        if(threadInfo[i].state == finished){
                            threadInfo[i].state = ready; // vlakno v threadPoolu je pripraveno
                            layers[cl + 1].queue.push_back(layers[cl + 1].size++); //thread na ite vrstve skonci, takze se uvolni misto pro i+1 vrstvu
                            layers[cl + 1].awaiting += 1; // o jeden seznam dokumentu vice ceka
                            threadPool[i].join();
                            //TODO vycistit predchozi mezivysledek
                        }
                        
                        if(threadInfo[i].state == ready){
                            //TODO jsou tam vsude +1 ???
                            if(layers[cl + 1].data.size() % 2 == 1 && layers[cl].first){ // pokud je licha TODO divne (cl+1 a size)
                                
                                if(layers[threadInfo[i].layer + 1].awaiting >= 3){
                                    auto i1 = layers[cl + 1].queue.back();
                                    layers[cl + 1].queue.back();
                                    auto i2 = layers[cl + 1].queue.back();
                                    layers[cl + 1].queue.back();
                                    auto i3 = layers[cl + 1].queue.back();
                                    layers[cl + 1].queue.back();
                                    
                                    auto fb = putFeaturesInBunch(layers[cl + 1].data[i1], layers[cl + 1].data[i2], layers[cl + 1].data[i3]);
                                    
                                    threadInfo[i].layer++;
                                    threadInfo[i].state = running;
                                    threadInfo[i].threadId = i; //TODO useless
                                    
                                    
                                    threadPool[i] = std::thread(&searchThead<Objects>, std::ref(layers[cl + 2].data[layers[cl + 2].size]), std::ref(fb.stateForFeature), std::ref(threadInfo[i].state)); //TODO currentCOunt ok?
                                    
                                    currentCount++;
                                    layers[cl + 1].first = false;
                                    layers[cl + 1].awaiting = layers[cl + 1].awaiting - 3;
                                }
                            }
                            
                            else if(layers[threadInfo[i].layer + 1].awaiting >= 2){ //pokud je suda
                                
                                auto i1 = layers[cl + 1].queue.back();
                                layers[cl + 1].queue.back();
                                auto i2 = layers[cl + 1].queue.back();
                                layers[cl + 1].queue.back();
                                
                                auto fb = putFeaturesInBunch(layers[cl + 1].data[i1], layers[cl + 1].data[i2]);
                                
                                threadInfo[i].layer++;
                                threadInfo[i].state = running;
                                threadInfo[i].threadId = i; //TODO useless
                                
                                
                                threadPool[i] = std::thread(searchThead<Objects>, std::ref(layers[cl + 2].data[layers[cl + 2].size]), std::ref(fb.stateForFeature), std::ref(threadInfo[i].state)); //TODO currentCOunt ok?
                                
                                currentCount++;
                                
                                layers[cl + 1].first = false;
                                layers[cl + 1].awaiting = layers[cl + 1].awaiting - 2;
                            }
                        }
                    }
                }
            }
            return layers[layersNr-1].data[0]; //never happens
        }
        
        const std::vector<uint64_t> & getDocuments() const {
            return documents;
        }
        
    private:
        void clear(){
            featureBunches.clear();
            documents.clear();
        }
        
//        template<typesname U>
        FeatureBunch<Objects> putFeaturesInBunch(FeatureEncloser<Objects> & f1, FeatureEncloser<Objects> & f2 ){
            FeatureBunch<Objects> fb;
            fb.addFeature(std::forward<FeatureEncloser<Objects>>(f1));
            fb.addFeature(std::forward<FeatureEncloser<Objects>>(f2));
            return fb;
        }
        
        FeatureBunch<Objects> putFeaturesInBunch(FeatureEncloser<Objects> & f1, FeatureEncloser<Objects> & f2, FeatureEncloser<Objects> & f3){
            FeatureBunch<Objects> fb;
            fb.addFeature(std::forward<FeatureEncloser<Objects>>(f1));
            fb.addFeature(std::forward<FeatureEncloser<Objects>>(f2));
            fb.addFeature(std::forward<FeatureEncloser<Objects>>(f3));
            return fb;
        }

        
        
        void splitToThreads(){
            size_t i = 0;
            size_t count = 0;
            clear();
            auto groups = std::min(threads, features2.size()/2);
            for(auto && f:features2){
                if(i < groups){
                    featureBunches.push_back(FeatureBunch<T>());
                    documents.push_back(FeatureEncloser<Objects>(f.getId()));
                }
                featureBunches[i % groups].addFeature(std::forward<FeatureEncloser<T>>(f));
                ++i;
            }
        }
        
        std::vector<FeatureEncloser<Objects>> documents;
        std::vector<FeatureBunch<T>> featureBunches;
        std::vector<FeatureEncloser<T>> features2;
        size_t featuresCount;
    };
}


namespace ii{
    using namespace ii_internal;
    
    template<typename Truncate, typename FeatureObjectLists>
    void create (Truncate&& truncate, FeatureObjectLists&& features){
        
        uint64_t sizeOfContainer = 0, nrOfFeatures = 0;
        uint64_t sizeOfIndex = 0;
        
        nrOfFeatures = features.size();
        sizeOfContainer += sizeof(uint64_t)/8; //ulozeni poctu featur
        
        sizeOfIndex = features.size() * 3 * sizeof(uint64_t) / 8;
        
        //int size optimized
        sizeOfIndex = features.size() * 10 * sizeof(uint64_t) / 8;
        // size je pocet prvku
        //    id | size (in int64 blocks) | size1 | offset1 | size2 | offset2 | size4 | offset4  | size8 | offset8
        
        //zacatek oblasti s XB, 1..0, 2..1, 4..2, 8..3:    i + 1 + *(i + 1 + 10*f + 2 + K*2 + 1)
        //pocet prvku oblasti s XB, 1..0, 2..1, 4..2, 8..3:    i + 1 + *(i + 1 + 10*f + 2 + K*2)
        //konec oblasti (krome 8) s XB, 1..0, 2..1, 4..2, 8..3:    i + 1 + *(i + 1 + 10*f + 2 + (K+1)*2 + 1)
        
        
        //USELESS
        //zacatek featury (offset) je na                i + 1 + *(i + 1 + 7*f + 2)
        //zacatek seznamu s cisly velikosti 1 je na     offset + *(i + 1 + 7*f + 3)
        // 8B velikost segmentu velikosti 1: *(offset + *(i + 1 + 7*f + 3))/8 + ...plus 1 nebo 0
        //zacatek seznamu s cisly velikosti 2 je na     offset + *(i + 1 + 7*f + 3)

        sizeOfContainer += sizeOfIndex; //cislo featury, pocet objektu ve feature, offset
        
        std::vector<uint64_t> sizeOfFeature;
        std::vector<uint64_t> sizeTillFeature;
        std::vector<uint64_t> sizeTill1, sizeTill2, sizeTill4, sizeTill8;
        std::vector<uint64_t> sizeOf1, sizeOf2, sizeOf4, sizeOf8; //  velikost jednotlivych
        std::vector<uint64_t> b1, b2, b4, b8; // pocet jednotlivych dokumentu dane velikosti

        sizeOf1.push_back(0);
        sizeOf2.push_back(0);
        sizeOf4.push_back(0);
        sizeOf8.push_back(0);

        
        sizeTill1.push_back(0);
        sizeTill2.push_back(0);
        sizeTill4.push_back(0);
        sizeTill8.push_back(0);
        
        b1.push_back(0);
        b2.push_back(0);
        b4.push_back(0);
        b8.push_back(0);


        sizeOfFeature.push_back(0);
        sizeTillFeature.push_back(0);
        
        
        for(auto && o:features[0]){
            if(o <= UINT8_MAX)
                ++b1[0];
            else if(o <= UINT16_MAX)
                ++b2[0];
            else if(o <= UINT32_MAX)
                ++b4[0];
            else
                ++b8[0];
        }

        
        uint64_t plus1, plus2, plus4, plus8;

        sizeTill1[0] = 0;
        
        plus1 = (b1[0] % 8) > 0 ? 1 : 0;
        sizeTill2[0] = sizeTill1[0] + b1[0]/8 + plus1;
        
        sizeOf1[0] = b1[0]/8 + plus1;
        
        plus2 = (b2[0] % 4) > 0 ? 1 : 0;
        sizeTill4[0] = sizeTill2[0] + b2[0]/4 + plus1;
        
        sizeOf2[0] = b2[0]/4 + plus2;
        
        plus4 = (b4[0] % 2) > 0 ? 1 : 0;
        sizeTill8[0] = sizeTill4[0] + b4[0]/2 + plus1;
        
        sizeOf4[0] = b4[0]/2 + plus4;
        
        sizeOf8[0] = b8[0];

        sizeOfFeature[0] = sizeOf1[0] + sizeOf2[0] + sizeOf4[0] + sizeOf8[0];
        sizeTill1[0] = 0;
        sizeTillFeature[0] = 0;
        
        sizeOfContainer+= sizeOfFeature[0];

        for(uint64_t i = 1; i < features.size(); ++i){
            sizeOfFeature.push_back(0);
            sizeTillFeature.push_back(0);
            
            sizeTill1.push_back(0);
            sizeTill2.push_back(0);
            sizeTill4.push_back(0);
            sizeTill8.push_back(0);
            
            sizeOf1.push_back(0);
            sizeOf2.push_back(0);
            sizeOf4.push_back(0);
            sizeOf8.push_back(0);
            
            
            b1.push_back(0);
            b2.push_back(0);
            b4.push_back(0);
            b8.push_back(0);
            for(auto && o:features[i]){
                if(o <= UINT8_MAX)
                    ++b1[i];
                else if(o <= UINT16_MAX)
                    ++b2[i];
                else if(o <= UINT32_MAX)
                    ++b4[i];
                else
                    ++b8[i];
            }

            sizeTill1[i] = sizeTillFeature[i-1] + sizeOfFeature[i-1];

            plus1 = (b1[i] % 8) > 0 ? 1 : 0;
            sizeOf1[i] = b1[i]/8 + plus1;
            sizeTill2[i] = sizeTill1[i] + b1[i]/8 + plus1;
            
            plus2 = (b2[i] % 4) > 0 ? 1 : 0;
            sizeOf2[i] = b2[i]/4 + plus2;
            sizeTill4[i] = sizeTill2[i] + b2[i]/4 + plus2;
            
            plus4 = (b4[i] % 2) > 0 ? 1 : 0;
            sizeOf4[i] = b4[i]/2 + plus4;
            sizeTill8[i] = sizeTill4[i] + b4[i]/2 + plus4;
            
            sizeOf8[i] = b8[i];

            sizeTillFeature[i] = sizeTillFeature[i-1] + sizeOfFeature[i-1];
            sizeOfFeature[i] = sizeOf1[i] + sizeOf2[i] + sizeOf4[i] + sizeOf8[i];
            sizeOfContainer += sizeOfFeature[i];
        }
        
        //container
        
        uint64_t * start = truncate(sizeOfContainer);
        uint64_t * currentPointer = start;
        uint64_t currentOffset = 0;
        
        //*(start + currentOffset++) = sizeOfContainer; // ulozeni velikosti celeho containeru
        *(start + currentOffset++) = nrOfFeatures;
        
        
        //zapisovani indexu
        for(uint64_t i = 0; i< nrOfFeatures; ++i){
            *(start + currentOffset++) = i; // cislo featury
            *(start + currentOffset++) = sizeOfFeature[i]; // pocet objektu ve feature
            *(start + currentOffset++) = b1[i]; //pocet 1B cisel
            *(start + currentOffset++) = sizeOfIndex + 1 + (sizeTill1[i]); // offset na zacatek oblasti
            *(start + currentOffset++) = b2[i]; //pocet 2B cisel
            *(start + currentOffset++) = sizeOfIndex + 1 + (sizeTill2[i]); // offset na zacatek oblasti
            *(start + currentOffset++) = b4[i]; //pocet 4B cisel
            *(start + currentOffset++) = sizeOfIndex + 1 + (sizeTill4[i]); // offset na zacatek oblasti
            *(start + currentOffset++) = b8[i]; //pocet 8B cisel
            *(start + currentOffset++) = sizeOfIndex + 1 + (sizeTill8[i]); // offset na zacatek oblasti

        }

        uint64_t f = 0;
        uint64_t size = 0;
        
        bool in1 = false, in2 = false,in4 =false, in8 =false;
        int8_t last = 1;
        for(uint64_t i = 0; i < features.size(); ++i){
            uint64_t sizeOfFeature = 0;
            for(auto && o:features[i]){
                if(o <= UINT8_MAX){
                    uint8_t * ptr = (uint8_t*) *(start + 1 + sizeOfIndex);
                    ptr += size;
                    *ptr = o;
                    last = 1;
                }
                else if(o <= UINT16_MAX){
                    if(last == 1){ //zarovnani na 8B
                        uint8_t plus = size%8 > 0 ? size%8 : 8;
                        size = size + 8 - plus;
                    }
                    uint16_t * ptr = (uint16_t*) *(start + 1 + sizeOfIndex);
                    ptr += size;
                    *ptr = o;
                    last = 2;
                }
                else if(o <= UINT32_MAX){
                    if(last == 2){
                        uint8_t plus = size%4 > 0 ? size%4 : 4;
                        size = size + 4 - plus;
                    }
                    uint32_t * ptr = (uint32_t*) *(start + 1 + sizeOfIndex);
                    ptr += size;
                    *ptr = o;
                    last = 4;
                }
                else{
                    if(last == 4){
                        uint8_t plus = size%2 > 0 ? size%2 : 2;
                        size = size + 2 - plus;
                    }
                    uint64_t * ptr = (uint64_t*) *(start + 1 + sizeOfIndex);
                    ptr += size;
                    *ptr = o;
                    last = 8;
                }
                sizeOfFeature += 1;
                size += 1;
            }
        }
        
    }
    


    template<class Fs, class OutFn>
    void search (const uint64_t* segment, size_t size, Fs&& fs, OutFn&& callback){
        
        std::vector<FeatureEncloser<FeatureOnDiskType>> features;
        for(auto && f:fs){
            features.push_back(FeatureEncloser<FeatureOnDiskType>(segment, f));
        }
        
        
        Finder<16, FeatureOnDiskType> finder(features);
        auto  docs = finder.search();
        for(auto && a:docs.data){
            print(a);
        }
        print("\n--------\n");

//        size_t t = 0;
//        std::vector<FeatureEncloser<Objects>> result;
//        for(auto && f:features){
//            result.push_back(FeatureEncloser<Objects>(f.getId()));
//            for(auto && doc:f.data){
//                result[t].data.push_back(doc);
//            }
//            ++t;
//        }
//
//        print("done\n");
//        Finder<16, Objects> finder(result);
//        auto docs = finder.search();
//        for(auto && a:docs.data){
//            print(a);
//        }
//        print("\n--------\n");


//        while(docs.size() > 1){
        
//        Finder<8, Objects> finder2(docs);
//        auto result = finder2.search();
//        docs = result;
        
        
//        }
        
//        for (auto && d:result[0].data) {
//            print(d);
////            callback(d);
//        }
        
    }

}; //namespace ii

#endif
