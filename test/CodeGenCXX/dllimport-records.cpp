// RUN: %clang_cc1 -triple i686-windows-msvc   -emit-llvm -std=c++1y -O0 -o - %s -DMSABI | FileCheck --check-prefix=MSC --check-prefix=M32 %s
// RUN: %clang_cc1 -triple x86_64-windows-msvc -emit-llvm -std=c++1y -O0 -o - %s -DMSABI | FileCheck --check-prefix=MSC --check-prefix=M64 %s
// RUN: %clang_cc1 -triple i686-windows-gnu    -emit-llvm -std=c++1y -O0 -o - %s         | FileCheck --check-prefix=GNU --check-prefix=G32 %s
// RUN: %clang_cc1 -triple x86_64-windows-gnu  -emit-llvm -std=c++1y -O0 -o - %s         | FileCheck --check-prefix=GNU --check-prefix=G64 %s
// RUN: %clang_cc1 -triple i686-windows-msvc   -emit-llvm -std=c++1y -O1 -o - %s -DMSABI | FileCheck --check-prefix=MO1 %s
// RUN: %clang_cc1 -triple i686-windows-gnu    -emit-llvm -std=c++1y -O1 -o - %s         | FileCheck --check-prefix=GO1 %s

// Helper structs to make templates more expressive.
struct ImplicitInst_Imported {};
struct ExplicitDecl_Imported {};
struct ExplicitInst_Imported {};
struct ExplicitSpec_Imported {};
struct ExplicitSpec_NotImported {};
struct MemberSpec_Imported;
namespace { struct Internal {}; }

#define JOIN2(x, y) x##y
#define JOIN(x, y) JOIN2(x, y)
#define UNIQ(name) JOIN(name, __LINE__)
#define USEVARTYPE(type, var) type UNIQ(use)() { return var; }
#define USEVAR(var) USEVARTYPE(int, var)
#define USE(func) void UNIQ(use)() { func(); }
#define USEMV(cls, var) int UNIQ(use)() { return ref(cls::var); }
#define USEMF(cls, fun) template<> void useMemFun<__LINE__, cls>() { cls().fun(); }
#define USEVMF(cls, fun) template<> void useMemFun<__LINE__, cls>() { cls().fun(); ptr<cls>()->fun(); }
#define USESPECIALS(cls) void UNIQ(use)() { useSpecials<cls>(); }
#define USEVIRTUAL(cls) void UNIQ(use)() { useVirtual<cls>(); }

namespace std {
  // Required for typeid().
  class type_info {};
}

template<typename T>
void useVirtual()
{
    delete new T();
    delete[] new T[1];
    (void)typeid(T);
}

template<typename T>
T ref(T const& v) { return v; }

template<typename T>
T* ptr();

template<int Line, typename T>
void useMemFun();

template<typename T>
void useSpecials() {
  T v; // Default constructor

  T c1(static_cast<const T&>(v)); // Copy constructor
  T c2 = static_cast<const T&>(v); // Copy constructor
  T c3;
  c3 = static_cast<const T&>(v); // Copy assignment

  T m1(static_cast<T&&>(v)); // Move constructor
  T m2 = static_cast<T&&>(v); // Move constructor
  T m3;
  m3 = static_cast<T&&>(v); // Move assignment
}

struct X {};

struct ForceNonTrivial {
  ForceNonTrivial();
  ~ForceNonTrivial();
  ForceNonTrivial(const ForceNonTrivial&);
  ForceNonTrivial& operator=(const ForceNonTrivial&);
  ForceNonTrivial(ForceNonTrivial&&);
  ForceNonTrivial& operator=(ForceNonTrivial&&);
};



//===----------------------------------------------------------------------===//
// Classes
//===----------------------------------------------------------------------===//

// Import non-dynamic class.
struct __declspec(dllimport) ImportClass {
  struct Nested {
    // M32-DAG: declare x86_thiscallcc void @"\01?normalDecl@Nested@ImportClass@@QAEXXZ"(%"struct.ImportClass::Nested"*)
    // M64-DAG: declare                void @"\01?normalDecl@Nested@ImportClass@@QEAAXXZ"(%"struct.ImportClass::Nested"*)
    // G32-DAG: declare x86_thiscallcc void @_ZN11ImportClass6Nested10normalDeclEv(%"struct.ImportClass::Nested"*)
    // G64-DAG: declare                void @_ZN11ImportClass6Nested10normalDeclEv(%"struct.ImportClass::Nested"*)
    void normalDecl();
  };

  struct __declspec(dllimport) ImportNested {
    // M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalDecl@ImportNested@ImportClass@@QAEXXZ"(%"struct.ImportClass::ImportNested"*)
    // M64-DAG: declare dllimport                void @"\01?normalDecl@ImportNested@ImportClass@@QEAAXXZ"(%"struct.ImportClass::ImportNested"*)
    // G32-DAG: declare dllimport x86_thiscallcc void @_ZN11ImportClass12ImportNested10normalDeclEv(%"struct.ImportClass::ImportNested"*)
    // G64-DAG: declare dllimport                void @_ZN11ImportClass12ImportNested10normalDeclEv(%"struct.ImportClass::ImportNested"*)
    void normalDecl();
  };

  // M32-DAG: define            x86_thiscallcc void @"\01?normalDef@ImportClass@@QAEXXZ"(%struct.ImportClass* %this)
  // M64-DAG: define                           void @"\01?normalDef@ImportClass@@QEAAXXZ"(%struct.ImportClass* %this)
  // M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalDecl@ImportClass@@QAEXXZ"(%struct.ImportClass*)
  // M64-DAG: declare dllimport                void @"\01?normalDecl@ImportClass@@QEAAXXZ"(%struct.ImportClass*)
  // M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalInclass@ImportClass@@QAEXXZ"(%struct.ImportClass*)
  // M64-DAG: declare dllimport                void @"\01?normalInclass@ImportClass@@QEAAXXZ"(%struct.ImportClass*)
  // M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalInlineDef@ImportClass@@QAEXXZ"(%struct.ImportClass*)
  // M64-DAG: declare dllimport                void @"\01?normalInlineDef@ImportClass@@QEAAXXZ"(%struct.ImportClass*)
  // M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalInlineDecl@ImportClass@@QAEXXZ"(%struct.ImportClass*)
  // M64-DAG: declare dllimport                void @"\01?normalInlineDecl@ImportClass@@QEAAXXZ"(%struct.ImportClass*)
  // G32-DAG: define            x86_thiscallcc void @_ZN11ImportClass9normalDefEv(%struct.ImportClass* %this)
  // G64-DAG: define                           void @_ZN11ImportClass9normalDefEv(%struct.ImportClass* %this)
  // G32-DAG: declare dllimport x86_thiscallcc void @_ZN11ImportClass10normalDeclEv(%struct.ImportClass*)
  // G64-DAG: declare dllimport                void @_ZN11ImportClass10normalDeclEv(%struct.ImportClass*)
  // G32-DAG: declare dllimport x86_thiscallcc void @_ZN11ImportClass13normalInclassEv(%struct.ImportClass*)
  // G64-DAG: declare dllimport                void @_ZN11ImportClass13normalInclassEv(%struct.ImportClass*)
  // G32-DAG: declare dllimport x86_thiscallcc void @_ZN11ImportClass15normalInlineDefEv(%struct.ImportClass*)
  // G64-DAG: declare dllimport                void @_ZN11ImportClass15normalInlineDefEv(%struct.ImportClass*)
  // G32-DAG: declare dllimport x86_thiscallcc void @_ZN11ImportClass16normalInlineDeclEv(%struct.ImportClass*)
  // G64-DAG: declare dllimport                void @_ZN11ImportClass16normalInlineDeclEv(%struct.ImportClass*)
  // MO1-DAG: define available_externally dllimport x86_thiscallcc void @"\01?normalInclass@ImportClass@@QAEXXZ"(
  // MO1-DAG: define available_externally dllimport x86_thiscallcc void @"\01?normalInlineDef@ImportClass@@QAEXXZ"(
  // MO1-DAG: define available_externally dllimport x86_thiscallcc void @"\01?normalInlineDecl@ImportClass@@QAEXXZ"(
  // GO1-DAG: define available_externally dllimport x86_thiscallcc void @_ZN11ImportClass13normalInclassEv(
  // GO1-DAG: define available_externally dllimport x86_thiscallcc void @_ZN11ImportClass15normalInlineDefEv(
  // GO1-DAG: define available_externally dllimport x86_thiscallcc void @_ZN11ImportClass16normalInlineDeclEv(
                void normalDef(); // dllimport ignored
                void normalDecl();
                void normalInclass() {}
                void normalInlineDef();
         inline void normalInlineDecl();

  // MSC-DAG: define                           void @"\01?staticDef@ImportClass@@SAXXZ"()
  // MSC-DAG: declare dllimport                void @"\01?staticDecl@ImportClass@@SAXXZ"()
  // MSC-DAG: declare dllimport                void @"\01?staticInclass@ImportClass@@SAXXZ"()
  // MSC-DAG: declare dllimport                void @"\01?staticInlineDef@ImportClass@@SAXXZ"()
  // MSC-DAG: declare dllimport                void @"\01?staticInlineDecl@ImportClass@@SAXXZ"()
  // GNU-DAG: define                           void @_ZN11ImportClass9staticDefEv()
  // GNU-DAG: declare dllimport                void @_ZN11ImportClass10staticDeclEv()
  // GNU-DAG: declare dllimport                void @_ZN11ImportClass13staticInclassEv()
  // GNU-DAG: declare dllimport                void @_ZN11ImportClass15staticInlineDefEv()
  // GNU-DAG: declare dllimport                void @_ZN11ImportClass16staticInlineDeclEv()
  // MO1-DAG: define available_externally dllimport void @"\01?staticInclass@ImportClass@@SAXXZ"()
  // MO1-DAG: define available_externally dllimport void @"\01?staticInlineDef@ImportClass@@SAXXZ"()
  // MO1-DAG: define available_externally dllimport void @"\01?staticInlineDecl@ImportClass@@SAXXZ"()
  // GO1-DAG: define available_externally dllimport void @_ZN11ImportClass13staticInclassEv()
  // GO1-DAG: define available_externally dllimport void @_ZN11ImportClass15staticInlineDefEv()
  // GO1-DAG: define available_externally dllimport void @_ZN11ImportClass16staticInlineDeclEv()
  static        void staticDef(); // dllimport ignored
  static        void staticDecl();
  static        void staticInclass() {}
  static        void staticInlineDef();
  static inline void staticInlineDecl();

  // M32-DAG: declare dllimport x86_thiscallcc void @"\01?protectedNormalDecl@ImportClass@@IAEXXZ"(%struct.ImportClass*)
  // M64-DAG: declare dllimport                void @"\01?protectedNormalDecl@ImportClass@@IEAAXXZ"(%struct.ImportClass*)
  // G32-DAG: declare dllimport x86_thiscallcc void @_ZN11ImportClass19protectedNormalDeclEv(%struct.ImportClass*)
  // G64-DAG: declare dllimport                void @_ZN11ImportClass19protectedNormalDeclEv(%struct.ImportClass*)
  // MSC-DAG: declare dllimport                void @"\01?protectedStaticDecl@ImportClass@@KAXXZ"()
  // GNU-DAG: declare dllimport                void @_ZN11ImportClass19protectedStaticDeclEv()
protected:
                void protectedNormalDecl();
  static        void protectedStaticDecl();

  // M32-DAG: declare dllimport x86_thiscallcc void @"\01?privateNormalDecl@ImportClass@@AAEXXZ"(%struct.ImportClass*)
  // M64-DAG: declare dllimport                void @"\01?privateNormalDecl@ImportClass@@AEAAXXZ"(%struct.ImportClass*)
  // G32-DAG: declare dllimport x86_thiscallcc void @_ZN11ImportClass17privateNormalDeclEv(%struct.ImportClass*)
  // G64-DAG: declare dllimport                void @_ZN11ImportClass17privateNormalDeclEv(%struct.ImportClass*)
  // MSC-DAG: declare dllimport                void @"\01?privateStaticDecl@ImportClass@@CAXXZ"()
  // GNU-DAG: declare dllimport                void @_ZN11ImportClass17privateStaticDeclEv()
private:
                void privateNormalDecl();
  static        void privateStaticDecl();

public:

  // MSC-DAG: @"\01?StaticField@ImportClass@@2HA"               = external dllimport global i32
  // MSC-DAG: @"\01?StaticConstField@ImportClass@@2HB"          = external dllimport constant i32
  // MSC-DAG: @"\01?StaticConstFieldEqualInit@ImportClass@@2HB" = available_externally dllimport constant i32 1, align 4
  // MSC-DAG: @"\01?StaticConstFieldBraceInit@ImportClass@@2HB" = available_externally dllimport constant i32 1, align 4
  // MSC-DAG: @"\01?ConstexprField@ImportClass@@2HB"            = available_externally dllimport constant i32 1, align 4
  // GNU-DAG: @_ZN11ImportClass11StaticFieldE                   = external dllimport global i32
  // GNU-DAG: @_ZN11ImportClass16StaticConstFieldE              = external dllimport constant i32
  // GNU-DAG: @_ZN11ImportClass25StaticConstFieldEqualInitE     = external dllimport constant i32
  // GNU-DAG: @_ZN11ImportClass25StaticConstFieldBraceInitE     = external dllimport constant i32
  // GNU-DAG: @_ZN11ImportClass14ConstexprFieldE                = external dllimport constant i32
                int  Field;
  static        int  StaticField;
  static const  int  StaticConstField;
  static const  int  StaticConstFieldEqualInit = 1;
  static const  int  StaticConstFieldBraceInit{1};
  constexpr static int ConstexprField = 1;

  template<int Line, typename T> friend void useMemFun();
};

       void ImportClass::normalDef() {} // dllimport ignored
inline void ImportClass::normalInlineDef() {}
       void ImportClass::normalInlineDecl() {}
       void ImportClass::staticDef() {} // dllimport ignored
inline void ImportClass::staticInlineDef() {}
       void ImportClass::staticInlineDecl() {}

USEMF(ImportClass::Nested, normalDecl)
USEMF(ImportClass::ImportNested, normalDecl)

USEMF(ImportClass, normalDef)
USEMF(ImportClass, normalDecl)
USEMF(ImportClass, normalInclass)
USEMF(ImportClass, normalInlineDef)
USEMF(ImportClass, normalInlineDecl)
USEMF(ImportClass, staticDef)
USEMF(ImportClass, staticDecl)
USEMF(ImportClass, staticInclass)
USEMF(ImportClass, staticInlineDef)
USEMF(ImportClass, staticInlineDecl)
USEMF(ImportClass, protectedNormalDecl)
USEMF(ImportClass, protectedStaticDecl)
USEMF(ImportClass, privateNormalDecl)
USEMF(ImportClass, privateStaticDecl)

USEMV(ImportClass, StaticField)
USEMV(ImportClass, StaticConstField)
USEMV(ImportClass, StaticConstFieldEqualInit)
USEMV(ImportClass, StaticConstFieldBraceInit)
USEMV(ImportClass, ConstexprField)


// Import class with special member functions.
struct __declspec(dllimport) ImportClassSpecials {
  // M32-DAG: declare dllimport x86_thiscallcc %struct.ImportClassSpecials* @"\01??0ImportClassSpecials@@QAE@XZ"(%struct.ImportClassSpecials* returned)
  // M64-DAG: declare dllimport                %struct.ImportClassSpecials* @"\01??0ImportClassSpecials@@QEAA@XZ"(%struct.ImportClassSpecials* returned)
  // G32-DAG: declare dllimport x86_thiscallcc void @_ZN19ImportClassSpecialsC1Ev(%struct.ImportClassSpecials*)
  // G64-DAG: declare dllimport                void @_ZN19ImportClassSpecialsC1Ev(%struct.ImportClassSpecials*)
  ImportClassSpecials();

  // M32-DAG: declare dllimport x86_thiscallcc void @"\01??1ImportClassSpecials@@QAE@XZ"(%struct.ImportClassSpecials*)
  // M64-DAG: declare dllimport                void @"\01??1ImportClassSpecials@@QEAA@XZ"(%struct.ImportClassSpecials*)
  // G32-DAG: declare dllimport x86_thiscallcc void @_ZN19ImportClassSpecialsD1Ev(%struct.ImportClassSpecials*)
  // G64-DAG: declare dllimport                void @_ZN19ImportClassSpecialsD1Ev(%struct.ImportClassSpecials*)
  ~ImportClassSpecials();

  // M32-DAG: declare dllimport x86_thiscallcc %struct.ImportClassSpecials* @"\01??0ImportClassSpecials@@QAE@ABU0@@Z"(%struct.ImportClassSpecials* returned, %struct.ImportClassSpecials*)
  // M64-DAG: declare dllimport                %struct.ImportClassSpecials* @"\01??0ImportClassSpecials@@QEAA@AEBU0@@Z"(%struct.ImportClassSpecials* returned, %struct.ImportClassSpecials*)
  // G32-DAG: declare dllimport x86_thiscallcc void @_ZN19ImportClassSpecialsC1ERKS_(%struct.ImportClassSpecials*, %struct.ImportClassSpecials*)
  // G64-DAG: declare dllimport                void @_ZN19ImportClassSpecialsC1ERKS_(%struct.ImportClassSpecials*, %struct.ImportClassSpecials*)
  ImportClassSpecials(const ImportClassSpecials&);

  // M32-DAG: declare dllimport x86_thiscallcc %struct.ImportClassSpecials* @"\01??4ImportClassSpecials@@QAEAAU0@ABU0@@Z"(%struct.ImportClassSpecials*, %struct.ImportClassSpecials*)
  // M64-DAG: declare dllimport                %struct.ImportClassSpecials* @"\01??4ImportClassSpecials@@QEAAAEAU0@AEBU0@@Z"(%struct.ImportClassSpecials*, %struct.ImportClassSpecials*)
  // G32-DAG: declare dllimport x86_thiscallcc %struct.ImportClassSpecials* @_ZN19ImportClassSpecialsaSERKS_(%struct.ImportClassSpecials*, %struct.ImportClassSpecials*)
  // G64-DAG: declare dllimport                %struct.ImportClassSpecials* @_ZN19ImportClassSpecialsaSERKS_(%struct.ImportClassSpecials*, %struct.ImportClassSpecials*)
  ImportClassSpecials& operator=(const ImportClassSpecials&);

  // M32-DAG: declare dllimport x86_thiscallcc %struct.ImportClassSpecials* @"\01??0ImportClassSpecials@@QAE@$$QAU0@@Z"(%struct.ImportClassSpecials* returned, %struct.ImportClassSpecials*)
  // M64-DAG: declare dllimport                %struct.ImportClassSpecials* @"\01??0ImportClassSpecials@@QEAA@$$QEAU0@@Z"(%struct.ImportClassSpecials* returned, %struct.ImportClassSpecials*)
  // G32-DAG: declare dllimport x86_thiscallcc void @_ZN19ImportClassSpecialsC1EOS_(%struct.ImportClassSpecials*, %struct.ImportClassSpecials*)
  // G64-DAG: declare dllimport                void @_ZN19ImportClassSpecialsC1EOS_(%struct.ImportClassSpecials*, %struct.ImportClassSpecials*)
  ImportClassSpecials(ImportClassSpecials&&);

  // M32-DAG: declare dllimport x86_thiscallcc %struct.ImportClassSpecials* @"\01??4ImportClassSpecials@@QAEAAU0@$$QAU0@@Z"(%struct.ImportClassSpecials*, %struct.ImportClassSpecials*)
  // M64-DAG: declare dllimport                %struct.ImportClassSpecials* @"\01??4ImportClassSpecials@@QEAAAEAU0@$$QEAU0@@Z"(%struct.ImportClassSpecials*, %struct.ImportClassSpecials*)
  // G32-DAG: declare dllimport x86_thiscallcc %struct.ImportClassSpecials* @_ZN19ImportClassSpecialsaSEOS_(%struct.ImportClassSpecials*, %struct.ImportClassSpecials*)
  // G64-DAG: declare dllimport                %struct.ImportClassSpecials* @_ZN19ImportClassSpecialsaSEOS_(%struct.ImportClassSpecials*, %struct.ImportClassSpecials*)
  ImportClassSpecials& operator=(ImportClassSpecials&&);
};
USESPECIALS(ImportClassSpecials)


// Implicitly defined trivial special member functions are not imported.
// FIXME: FileCheck complains about reordering DAG across NOT. Without
// true global NOT checks this might require a separate test file.
// MSC-NOT-DISABLED: \01??1ImportTrivialImplicitSpecials
// GNU-NOT-DISABLED: _ZN21ImportTrivialImplicitSpecials
struct __declspec(dllimport) ImportTrivialSpecials {
};
USESPECIALS(ImportTrivialSpecials)


// Implicitly defined non-trivial special member functions are imported.
//
// Default constructor
// M32-DAG: declare dllimport x86_thiscallcc %struct.ImportComplexSpecials* @"\01??0ImportComplexSpecials@@QAE@XZ"(%struct.ImportComplexSpecials* returned)
// M64-DAG: declare dllimport                %struct.ImportComplexSpecials* @"\01??0ImportComplexSpecials@@QEAA@XZ"(%struct.ImportComplexSpecials* returned)
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN21ImportComplexSpecialsC1Ev(%struct.ImportComplexSpecials*)
// G64-DAG: declare dllimport                void @_ZN21ImportComplexSpecialsC1Ev(%struct.ImportComplexSpecials*)
// MO1-DAG: define available_externally dllimport x86_thiscallcc %struct.ImportComplexSpecials* @"\01??0ImportComplexSpecials@@QAE@XZ"(
// GO1-DAG: define available_externally dllimport x86_thiscallcc void @_ZN21ImportComplexSpecialsC1Ev(
//
// Destructor
// M32-DAG: declare dllimport x86_thiscallcc void @"\01??1ImportComplexSpecials@@QAE@XZ"(%struct.ImportComplexSpecials*)
// M64-DAG: declare dllimport                void @"\01??1ImportComplexSpecials@@QEAA@XZ"(%struct.ImportComplexSpecials*)
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN21ImportComplexSpecialsD1Ev(%struct.ImportComplexSpecials*)
// G64-DAG: declare dllimport                void @_ZN21ImportComplexSpecialsD1Ev(%struct.ImportComplexSpecials*)
// MO1-DAG: define available_externally dllimport x86_thiscallcc void @"\01??1ImportComplexSpecials@@QAE@XZ"(
// GO1-DAG: define available_externally dllimport x86_thiscallcc void @_ZN21ImportComplexSpecialsD1Ev(
//
// Copy constructor
// M32-DAG: declare dllimport x86_thiscallcc %struct.ImportComplexSpecials* @"\01??0ImportComplexSpecials@@QAE@ABU0@@Z"(%struct.ImportComplexSpecials* returned, %struct.ImportComplexSpecials*)
// M64-DAG: declare dllimport                %struct.ImportComplexSpecials* @"\01??0ImportComplexSpecials@@QEAA@AEBU0@@Z"(%struct.ImportComplexSpecials* returned, %struct.ImportComplexSpecials*)
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN21ImportComplexSpecialsC1ERKS_(%struct.ImportComplexSpecials*, %struct.ImportComplexSpecials*)
// G64-DAG: declare dllimport                void @_ZN21ImportComplexSpecialsC1ERKS_(%struct.ImportComplexSpecials*, %struct.ImportComplexSpecials*)
// MO1-DAG: define available_externally dllimport x86_thiscallcc %struct.ImportComplexSpecials* @"\01??0ImportComplexSpecials@@QAE@ABU0@@Z"(
// GO1-DAG: define available_externally dllimport x86_thiscallcc void @_ZN21ImportComplexSpecialsC1ERKS_(
//
// Copy assignment
// M32-DAG: declare dllimport x86_thiscallcc %struct.ImportComplexSpecials* @"\01??4ImportComplexSpecials@@QAEAAU0@ABU0@@Z"(%struct.ImportComplexSpecials*, %struct.ImportComplexSpecials*)
// M64-DAG: declare dllimport                %struct.ImportComplexSpecials* @"\01??4ImportComplexSpecials@@QEAAAEAU0@AEBU0@@Z"(%struct.ImportComplexSpecials*, %struct.ImportComplexSpecials*)
// G32-DAG: declare dllimport x86_thiscallcc %struct.ImportComplexSpecials* @_ZN21ImportComplexSpecialsaSERKS_(%struct.ImportComplexSpecials*, %struct.ImportComplexSpecials*)
// G64-DAG: declare dllimport                %struct.ImportComplexSpecials* @_ZN21ImportComplexSpecialsaSERKS_(%struct.ImportComplexSpecials*, %struct.ImportComplexSpecials*)
// MO1-DAG: define available_externally dllimport x86_thiscallcc %struct.ImportComplexSpecials* @"\01??4ImportComplexSpecials@@QAEAAU0@ABU0@@Z"(
// GO1-DAG: define available_externally dllimport x86_thiscallcc %struct.ImportComplexSpecials* @_ZN21ImportComplexSpecialsaSERKS_(
//
// Move constructor
// M32-DAG: declare dllimport x86_thiscallcc %struct.ImportComplexSpecials* @"\01??0ImportComplexSpecials@@QAE@$$QAU0@@Z"(%struct.ImportComplexSpecials* returned, %struct.ImportComplexSpecials*)
// M64-DAG: declare dllimport                %struct.ImportComplexSpecials* @"\01??0ImportComplexSpecials@@QEAA@$$QEAU0@@Z"(%struct.ImportComplexSpecials* returned, %struct.ImportComplexSpecials*)
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN21ImportComplexSpecialsC1EOS_(%struct.ImportComplexSpecials*, %struct.ImportComplexSpecials*)
// G64-DAG: declare dllimport                void @_ZN21ImportComplexSpecialsC1EOS_(%struct.ImportComplexSpecials*, %struct.ImportComplexSpecials*)
// MO1-DAG: define available_externally dllimport x86_thiscallcc %struct.ImportComplexSpecials* @"\01??0ImportComplexSpecials@@QAE@$$QAU0@@Z"(
// GO1-DAG: define available_externally dllimport x86_thiscallcc void @_ZN21ImportComplexSpecialsC1EOS_(
//
// Move assignment
// M32-DAG: declare dllimport x86_thiscallcc %struct.ImportComplexSpecials* @"\01??4ImportComplexSpecials@@QAEAAU0@$$QAU0@@Z"(%struct.ImportComplexSpecials*, %struct.ImportComplexSpecials*)
// M64-DAG: declare dllimport                %struct.ImportComplexSpecials* @"\01??4ImportComplexSpecials@@QEAAAEAU0@$$QEAU0@@Z"(%struct.ImportComplexSpecials*, %struct.ImportComplexSpecials*)
// G32-DAG: declare dllimport x86_thiscallcc %struct.ImportComplexSpecials* @_ZN21ImportComplexSpecialsaSEOS_(%struct.ImportComplexSpecials*, %struct.ImportComplexSpecials*)
// G64-DAG: declare dllimport                %struct.ImportComplexSpecials* @_ZN21ImportComplexSpecialsaSEOS_(%struct.ImportComplexSpecials*, %struct.ImportComplexSpecials*)
// MO1-DAG: define available_externally dllimport x86_thiscallcc %struct.ImportComplexSpecials* @"\01??4ImportComplexSpecials@@QAEAAU0@$$QAU0@@Z"(
// GO1-DAG: define available_externally dllimport x86_thiscallcc %struct.ImportComplexSpecials* @_ZN21ImportComplexSpecialsaSEOS_(
struct __declspec(dllimport) ImportComplexSpecials {
  ForceNonTrivial v;
};
USESPECIALS(ImportComplexSpecials)


// Import class with inline special member functions.
struct __declspec(dllimport) ImportInlineSpecials {
  // M32-DAG: declare dllimport x86_thiscallcc %struct.ImportInlineSpecials* @"\01??0ImportInlineSpecials@@QAE@XZ"(%struct.ImportInlineSpecials* returned)
  // M64-DAG: declare dllimport                %struct.ImportInlineSpecials* @"\01??0ImportInlineSpecials@@QEAA@XZ"(%struct.ImportInlineSpecials* returned)
  // G32-DAG: declare dllimport x86_thiscallcc void @_ZN20ImportInlineSpecialsC1Ev(%struct.ImportInlineSpecials*)
  // G64-DAG: declare dllimport                void @_ZN20ImportInlineSpecialsC1Ev(%struct.ImportInlineSpecials*)
  // MO1-DAG: define available_externally dllimport x86_thiscallcc %struct.ImportInlineSpecials* @"\01??0ImportInlineSpecials@@QAE@XZ"(
  // GO1-DAG: define available_externally dllimport x86_thiscallcc void @_ZN20ImportInlineSpecialsC1Ev(
  ImportInlineSpecials() {}

  // M32-DAG: declare dllimport x86_thiscallcc void @"\01??1ImportInlineSpecials@@QAE@XZ"(%struct.ImportInlineSpecials*)
  // M64-DAG: declare dllimport                void @"\01??1ImportInlineSpecials@@QEAA@XZ"(%struct.ImportInlineSpecials*)
  // G32-DAG: declare dllimport x86_thiscallcc void @_ZN20ImportInlineSpecialsD1Ev(%struct.ImportInlineSpecials*)
  // G64-DAG: declare dllimport                void @_ZN20ImportInlineSpecialsD1Ev(%struct.ImportInlineSpecials*)
  // MO1-DAG: define available_externally dllimport x86_thiscallcc void @"\01??1ImportInlineSpecials@@QAE@XZ"(
  // GO1-DAG: define available_externally dllimport x86_thiscallcc void @_ZN20ImportInlineSpecialsD1Ev(
  ~ImportInlineSpecials() {}

  // M32-DAG: declare dllimport x86_thiscallcc %struct.ImportInlineSpecials* @"\01??0ImportInlineSpecials@@QAE@ABU0@@Z"(%struct.ImportInlineSpecials* returned, %struct.ImportInlineSpecials*)
  // M64-DAG: declare dllimport                %struct.ImportInlineSpecials* @"\01??0ImportInlineSpecials@@QEAA@AEBU0@@Z"(%struct.ImportInlineSpecials* returned, %struct.ImportInlineSpecials*)
  // G32-DAG: declare dllimport x86_thiscallcc void @_ZN20ImportInlineSpecialsC1ERKS_(%struct.ImportInlineSpecials*, %struct.ImportInlineSpecials*)
  // G64-DAG: declare dllimport                void @_ZN20ImportInlineSpecialsC1ERKS_(%struct.ImportInlineSpecials*, %struct.ImportInlineSpecials*)
  // MO1-DAG: define available_externally dllimport x86_thiscallcc %struct.ImportInlineSpecials* @"\01??0ImportInlineSpecials@@QAE@ABU0@@Z"(
  // GO1-DAG: define available_externally dllimport x86_thiscallcc void @_ZN20ImportInlineSpecialsC1ERKS_(
  inline ImportInlineSpecials(const ImportInlineSpecials&);

  // M32-DAG: declare dllimport x86_thiscallcc %struct.ImportInlineSpecials* @"\01??4ImportInlineSpecials@@QAEAAU0@ABU0@@Z"(%struct.ImportInlineSpecials*, %struct.ImportInlineSpecials*)
  // M64-DAG: declare dllimport                %struct.ImportInlineSpecials* @"\01??4ImportInlineSpecials@@QEAAAEAU0@AEBU0@@Z"(%struct.ImportInlineSpecials*, %struct.ImportInlineSpecials*)
  // G32-DAG: declare dllimport x86_thiscallcc %struct.ImportInlineSpecials* @_ZN20ImportInlineSpecialsaSERKS_(%struct.ImportInlineSpecials*, %struct.ImportInlineSpecials*)
  // G64-DAG: declare dllimport                %struct.ImportInlineSpecials* @_ZN20ImportInlineSpecialsaSERKS_(%struct.ImportInlineSpecials*, %struct.ImportInlineSpecials*)
  // MO1-DAG: define available_externally dllimport x86_thiscallcc %struct.ImportInlineSpecials* @"\01??4ImportInlineSpecials@@QAEAAU0@ABU0@@Z"(
  // GO1-DAG: define available_externally dllimport x86_thiscallcc %struct.ImportInlineSpecials* @_ZN20ImportInlineSpecialsaSERKS_(
  ImportInlineSpecials& operator=(const ImportInlineSpecials&);

  // M32-DAG: declare dllimport x86_thiscallcc %struct.ImportInlineSpecials* @"\01??0ImportInlineSpecials@@QAE@$$QAU0@@Z"(%struct.ImportInlineSpecials* returned, %struct.ImportInlineSpecials*)
  // M64-DAG: declare dllimport                %struct.ImportInlineSpecials* @"\01??0ImportInlineSpecials@@QEAA@$$QEAU0@@Z"(%struct.ImportInlineSpecials* returned, %struct.ImportInlineSpecials*)
  // G32-DAG: declare dllimport x86_thiscallcc void @_ZN20ImportInlineSpecialsC1EOS_(%struct.ImportInlineSpecials*, %struct.ImportInlineSpecials*)
  // G64-DAG: declare dllimport                void @_ZN20ImportInlineSpecialsC1EOS_(%struct.ImportInlineSpecials*, %struct.ImportInlineSpecials*)
  // MO1-DAG: define available_externally dllimport x86_thiscallcc %struct.ImportInlineSpecials* @"\01??0ImportInlineSpecials@@QAE@$$QAU0@@Z"(
  // GO1-DAG: define available_externally dllimport x86_thiscallcc void @_ZN20ImportInlineSpecialsC1EOS_(
  ImportInlineSpecials(ImportInlineSpecials&&) {}

  // M32-DAG: declare dllimport x86_thiscallcc %struct.ImportInlineSpecials* @"\01??4ImportInlineSpecials@@QAEAAU0@$$QAU0@@Z"(%struct.ImportInlineSpecials*, %struct.ImportInlineSpecials*)
  // M64-DAG: declare dllimport                %struct.ImportInlineSpecials* @"\01??4ImportInlineSpecials@@QEAAAEAU0@$$QEAU0@@Z"(%struct.ImportInlineSpecials*, %struct.ImportInlineSpecials*)
  // G32-DAG: declare dllimport x86_thiscallcc %struct.ImportInlineSpecials* @_ZN20ImportInlineSpecialsaSEOS_(%struct.ImportInlineSpecials*, %struct.ImportInlineSpecials*)
  // G64-DAG: declare dllimport                %struct.ImportInlineSpecials* @_ZN20ImportInlineSpecialsaSEOS_(%struct.ImportInlineSpecials*, %struct.ImportInlineSpecials*)
  // MO1-DAG: define available_externally dllimport x86_thiscallcc %struct.ImportInlineSpecials* @"\01??4ImportInlineSpecials@@QAEAAU0@$$QAU0@@Z"(
  // GO1-DAG: define available_externally dllimport x86_thiscallcc %struct.ImportInlineSpecials* @_ZN20ImportInlineSpecialsaSEOS_(
  ImportInlineSpecials& operator=(ImportInlineSpecials&&) { return *this; }
};
ImportInlineSpecials::ImportInlineSpecials(const ImportInlineSpecials&) {}
inline ImportInlineSpecials& ImportInlineSpecials::operator=(const ImportInlineSpecials&) { return *this; }
USESPECIALS(ImportInlineSpecials)


// Import class with defaulted member functions.
struct __declspec(dllimport) ImportClassDefaulted {
  // M32-DAG: declare dllimport x86_thiscallcc %struct.ImportClassDefaulted* @"\01??0ImportClassDefaulted@@QAE@XZ"(%struct.ImportClassDefaulted* returned)
  // M64-DAG: declare dllimport                %struct.ImportClassDefaulted* @"\01??0ImportClassDefaulted@@QEAA@XZ"(%struct.ImportClassDefaulted* returned)
  // G32-DAG: declare dllimport x86_thiscallcc void @_ZN20ImportClassDefaultedC1Ev(%struct.ImportClassDefaulted*)
  // G64-DAG: declare dllimport                void @_ZN20ImportClassDefaultedC1Ev(%struct.ImportClassDefaulted*)
  // MO1-DAG: define available_externally dllimport x86_thiscallcc %struct.ImportClassDefaulted* @"\01??0ImportClassDefaulted@@QAE@XZ"(%struct.ImportClassDefaulted* returned %this)
  // GO1-DAG: define available_externally dllimport x86_thiscallcc void @_ZN20ImportClassDefaultedC1Ev(%struct.ImportClassDefaulted* %this)
  ImportClassDefaulted() = default;

  // M32-DAG: declare dllimport x86_thiscallcc void @"\01??1ImportClassDefaulted@@QAE@XZ"(%struct.ImportClassDefaulted*)
  // M64-DAG: declare dllimport                void @"\01??1ImportClassDefaulted@@QEAA@XZ"(%struct.ImportClassDefaulted*)
  // G32-DAG: declare dllimport x86_thiscallcc void @_ZN20ImportClassDefaultedD1Ev(%struct.ImportClassDefaulted*)
  // G64-DAG: declare dllimport                void @_ZN20ImportClassDefaultedD1Ev(%struct.ImportClassDefaulted*)
  // MO1-DAG: define available_externally dllimport x86_thiscallcc void @"\01??1ImportClassDefaulted@@QAE@XZ"(%struct.ImportClassDefaulted* %this)
  // GO1-DAG: define available_externally dllimport x86_thiscallcc void @_ZN20ImportClassDefaultedD1Ev(%struct.ImportClassDefaulted* %this)
  ~ImportClassDefaulted() = default;

  // M32-DAG: declare dllimport x86_thiscallcc %struct.ImportClassDefaulted* @"\01??0ImportClassDefaulted@@QAE@ABU0@@Z"(%struct.ImportClassDefaulted* returned, %struct.ImportClassDefaulted*)
  // M64-DAG: declare dllimport                %struct.ImportClassDefaulted* @"\01??0ImportClassDefaulted@@QEAA@AEBU0@@Z"(%struct.ImportClassDefaulted* returned, %struct.ImportClassDefaulted*)
  // G32-DAG: declare dllimport x86_thiscallcc void @_ZN20ImportClassDefaultedC1ERKS_(%struct.ImportClassDefaulted*, %struct.ImportClassDefaulted*)
  // G64-DAG: declare dllimport                void @_ZN20ImportClassDefaultedC1ERKS_(%struct.ImportClassDefaulted*, %struct.ImportClassDefaulted*)
  // MO1-DAG: define available_externally dllimport x86_thiscallcc %struct.ImportClassDefaulted* @"\01??0ImportClassDefaulted@@QAE@ABU0@@Z"(%struct.ImportClassDefaulted* returned %this, %struct.ImportClassDefaulted*)
  // GO1-DAG: define available_externally dllimport x86_thiscallcc void @_ZN20ImportClassDefaultedC1ERKS_(%struct.ImportClassDefaulted* %this, %struct.ImportClassDefaulted*)
  ImportClassDefaulted(const ImportClassDefaulted&) = default;

  // M32-DAG: declare dllimport x86_thiscallcc %struct.ImportClassDefaulted* @"\01??4ImportClassDefaulted@@QAEAAU0@ABU0@@Z"(%struct.ImportClassDefaulted*, %struct.ImportClassDefaulted*)
  // M64-DAG: declare dllimport                %struct.ImportClassDefaulted* @"\01??4ImportClassDefaulted@@QEAAAEAU0@AEBU0@@Z"(%struct.ImportClassDefaulted*, %struct.ImportClassDefaulted*)
  // G32-DAG: declare dllimport x86_thiscallcc %struct.ImportClassDefaulted* @_ZN20ImportClassDefaultedaSERKS_(%struct.ImportClassDefaulted*, %struct.ImportClassDefaulted*)
  // G64-DAG: declare dllimport                %struct.ImportClassDefaulted* @_ZN20ImportClassDefaultedaSERKS_(%struct.ImportClassDefaulted*, %struct.ImportClassDefaulted*)
  // MO1-DAG: define available_externally dllimport x86_thiscallcc %struct.ImportClassDefaulted* @"\01??4ImportClassDefaulted@@QAEAAU0@ABU0@@Z"(%struct.ImportClassDefaulted* %this, %struct.ImportClassDefaulted*)
  // GO1-DAG: define available_externally dllimport x86_thiscallcc %struct.ImportClassDefaulted* @_ZN20ImportClassDefaultedaSERKS_(%struct.ImportClassDefaulted* %this, %struct.ImportClassDefaulted*)
  ImportClassDefaulted& operator=(const ImportClassDefaulted&) = default;

  // M32-DAG: declare dllimport x86_thiscallcc %struct.ImportClassDefaulted* @"\01??0ImportClassDefaulted@@QAE@$$QAU0@@Z"(%struct.ImportClassDefaulted* returned, %struct.ImportClassDefaulted*)
  // M64-DAG: declare dllimport                %struct.ImportClassDefaulted* @"\01??0ImportClassDefaulted@@QEAA@$$QEAU0@@Z"(%struct.ImportClassDefaulted* returned, %struct.ImportClassDefaulted*)
  // G32-DAG: declare dllimport x86_thiscallcc void @_ZN20ImportClassDefaultedC1EOS_(%struct.ImportClassDefaulted*, %struct.ImportClassDefaulted*)
  // G64-DAG: declare dllimport                void @_ZN20ImportClassDefaultedC1EOS_(%struct.ImportClassDefaulted*, %struct.ImportClassDefaulted*)
  // MO1-DAG: define available_externally dllimport x86_thiscallcc %struct.ImportClassDefaulted* @"\01??0ImportClassDefaulted@@QAE@$$QAU0@@Z"(%struct.ImportClassDefaulted* returned %this, %struct.ImportClassDefaulted*)
  // GO1-DAG: define available_externally dllimport x86_thiscallcc void @_ZN20ImportClassDefaultedC1EOS_(%struct.ImportClassDefaulted* %this, %struct.ImportClassDefaulted*)
  ImportClassDefaulted(ImportClassDefaulted&&) = default;

  // M32-DAG: declare dllimport x86_thiscallcc %struct.ImportClassDefaulted* @"\01??4ImportClassDefaulted@@QAEAAU0@$$QAU0@@Z"(%struct.ImportClassDefaulted*, %struct.ImportClassDefaulted*)
  // M64-DAG: declare dllimport                %struct.ImportClassDefaulted* @"\01??4ImportClassDefaulted@@QEAAAEAU0@$$QEAU0@@Z"(%struct.ImportClassDefaulted*, %struct.ImportClassDefaulted*)
  // G32-DAG: declare dllimport x86_thiscallcc %struct.ImportClassDefaulted* @_ZN20ImportClassDefaultedaSEOS_(%struct.ImportClassDefaulted*, %struct.ImportClassDefaulted*)
  // G64-DAG: declare dllimport                %struct.ImportClassDefaulted* @_ZN20ImportClassDefaultedaSEOS_(%struct.ImportClassDefaulted*, %struct.ImportClassDefaulted*)
  // MO1-DAG: define available_externally dllimport x86_thiscallcc %struct.ImportClassDefaulted* @"\01??4ImportClassDefaulted@@QAEAAU0@$$QAU0@@Z"(%struct.ImportClassDefaulted* %this, %struct.ImportClassDefaulted*)
  // GO1-DAG: define available_externally dllimport x86_thiscallcc %struct.ImportClassDefaulted* @_ZN20ImportClassDefaultedaSEOS_(%struct.ImportClassDefaulted* %this, %struct.ImportClassDefaulted*)
  ImportClassDefaulted& operator=(ImportClassDefaulted&&) = default;
  ForceNonTrivial v;
};
USESPECIALS(ImportClassDefaulted)


// Import class with defaulted member function definitions.
struct __declspec(dllimport) ImportClassDefaultedDefs {
  inline ImportClassDefaultedDefs();
  inline ~ImportClassDefaultedDefs();

  ImportClassDefaultedDefs(const ImportClassDefaultedDefs&);
  ImportClassDefaultedDefs& operator=(const ImportClassDefaultedDefs&);

  ImportClassDefaultedDefs(ImportClassDefaultedDefs&&);
  ImportClassDefaultedDefs& operator=(ImportClassDefaultedDefs&&);

  ForceNonTrivial v;
};
USESPECIALS(ImportClassDefaultedDefs)

// M32-DAG: declare dllimport x86_thiscallcc %struct.ImportClassDefaultedDefs* @"\01??0ImportClassDefaultedDefs@@QAE@XZ"(%struct.ImportClassDefaultedDefs* returned)
// M64-DAG: declare dllimport                %struct.ImportClassDefaultedDefs* @"\01??0ImportClassDefaultedDefs@@QEAA@XZ"(%struct.ImportClassDefaultedDefs* returned)
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN24ImportClassDefaultedDefsC1Ev(%struct.ImportClassDefaultedDefs*)
// G64-DAG: declare dllimport                void @_ZN24ImportClassDefaultedDefsC1Ev(%struct.ImportClassDefaultedDefs*)
ImportClassDefaultedDefs::ImportClassDefaultedDefs() = default;

// M32-DAG: declare dllimport x86_thiscallcc void @"\01??1ImportClassDefaultedDefs@@QAE@XZ"(%struct.ImportClassDefaultedDefs*)
// M64-DAG: declare dllimport                void @"\01??1ImportClassDefaultedDefs@@QEAA@XZ"(%struct.ImportClassDefaultedDefs*)
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN24ImportClassDefaultedDefsD1Ev(%struct.ImportClassDefaultedDefs*)
// G64-DAG: declare dllimport                void @_ZN24ImportClassDefaultedDefsD1Ev(%struct.ImportClassDefaultedDefs*)
ImportClassDefaultedDefs::~ImportClassDefaultedDefs() = default;

// M32-DAG: declare dllimport x86_thiscallcc %struct.ImportClassDefaultedDefs* @"\01??0ImportClassDefaultedDefs@@QAE@ABU0@@Z"(%struct.ImportClassDefaultedDefs* returned, %struct.ImportClassDefaultedDefs*)
// M64-DAG: declare dllimport                %struct.ImportClassDefaultedDefs* @"\01??0ImportClassDefaultedDefs@@QEAA@AEBU0@@Z"(%struct.ImportClassDefaultedDefs* returned, %struct.ImportClassDefaultedDefs*)
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN24ImportClassDefaultedDefsC1ERKS_(%struct.ImportClassDefaultedDefs*, %struct.ImportClassDefaultedDefs*)
// G64-DAG: declare dllimport                void @_ZN24ImportClassDefaultedDefsC1ERKS_(%struct.ImportClassDefaultedDefs*, %struct.ImportClassDefaultedDefs*)
inline ImportClassDefaultedDefs::ImportClassDefaultedDefs(const ImportClassDefaultedDefs&) = default;

// M32-DAG: declare dllimport x86_thiscallcc %struct.ImportClassDefaultedDefs* @"\01??4ImportClassDefaultedDefs@@QAEAAU0@ABU0@@Z"(%struct.ImportClassDefaultedDefs*, %struct.ImportClassDefaultedDefs*)
// M64-DAG: declare dllimport                %struct.ImportClassDefaultedDefs* @"\01??4ImportClassDefaultedDefs@@QEAAAEAU0@AEBU0@@Z"(%struct.ImportClassDefaultedDefs*, %struct.ImportClassDefaultedDefs*)
// G32-DAG: declare dllimport x86_thiscallcc %struct.ImportClassDefaultedDefs* @_ZN24ImportClassDefaultedDefsaSERKS_(%struct.ImportClassDefaultedDefs*, %struct.ImportClassDefaultedDefs*)
// G64-DAG: declare dllimport                %struct.ImportClassDefaultedDefs* @_ZN24ImportClassDefaultedDefsaSERKS_(%struct.ImportClassDefaultedDefs*, %struct.ImportClassDefaultedDefs*)
inline ImportClassDefaultedDefs& ImportClassDefaultedDefs::operator=(const ImportClassDefaultedDefs&) = default;

// M32-DAG: define x86_thiscallcc %struct.ImportClassDefaultedDefs* @"\01??0ImportClassDefaultedDefs@@QAE@$$QAU0@@Z"(%struct.ImportClassDefaultedDefs* returned %this, %struct.ImportClassDefaultedDefs*)
// M64-DAG: define                %struct.ImportClassDefaultedDefs* @"\01??0ImportClassDefaultedDefs@@QEAA@$$QEAU0@@Z"(%struct.ImportClassDefaultedDefs* returned %this, %struct.ImportClassDefaultedDefs*)
// G32-DAG: define x86_thiscallcc void @_ZN24ImportClassDefaultedDefsC1EOS_(%struct.ImportClassDefaultedDefs* %this, %struct.ImportClassDefaultedDefs*)
// G64-DAG: define                void @_ZN24ImportClassDefaultedDefsC1EOS_(%struct.ImportClassDefaultedDefs* %this, %struct.ImportClassDefaultedDefs*)
// G32-DAG: define x86_thiscallcc void @_ZN24ImportClassDefaultedDefsC2EOS_(%struct.ImportClassDefaultedDefs* %this, %struct.ImportClassDefaultedDefs*)
// G64-DAG: define                void @_ZN24ImportClassDefaultedDefsC2EOS_(%struct.ImportClassDefaultedDefs* %this, %struct.ImportClassDefaultedDefs*)
ImportClassDefaultedDefs::ImportClassDefaultedDefs(ImportClassDefaultedDefs&&) = default; // dllimport ignored

// M32-DAG: define x86_thiscallcc %struct.ImportClassDefaultedDefs* @"\01??4ImportClassDefaultedDefs@@QAEAAU0@$$QAU0@@Z"(%struct.ImportClassDefaultedDefs* %this, %struct.ImportClassDefaultedDefs*)
// M64-DAG: define                %struct.ImportClassDefaultedDefs* @"\01??4ImportClassDefaultedDefs@@QEAAAEAU0@$$QEAU0@@Z"(%struct.ImportClassDefaultedDefs* %this, %struct.ImportClassDefaultedDefs*)
// G32-DAG: define x86_thiscallcc %struct.ImportClassDefaultedDefs* @_ZN24ImportClassDefaultedDefsaSEOS_(%struct.ImportClassDefaultedDefs* %this, %struct.ImportClassDefaultedDefs*)
// G64-DAG: define                %struct.ImportClassDefaultedDefs* @_ZN24ImportClassDefaultedDefsaSEOS_(%struct.ImportClassDefaultedDefs* %this, %struct.ImportClassDefaultedDefs*)
ImportClassDefaultedDefs& ImportClassDefaultedDefs::operator=(ImportClassDefaultedDefs&&) = default; // dllimport ignored

USESPECIALS(ImportClassDefaultedDefs)

// Import class with allocation functions.
struct __declspec(dllimport) ImportClassAlloc {
  void* operator new(__SIZE_TYPE__);
  void* operator new[](__SIZE_TYPE__);
  void operator delete(void*);
  void operator delete[](void*);
};

// M32-DAG: declare dllimport i8* @"\01??2ImportClassAlloc@@SAPAXI@Z"(i32)
// M64-DAG: declare dllimport i8* @"\01??2ImportClassAlloc@@SAPEAX_K@Z"(i64)
// G32-DAG: declare dllimport i8* @_ZN16ImportClassAllocnwEj(i32)
// G64-DAG: declare dllimport i8* @_ZN16ImportClassAllocnwEy(i64)
void UNIQ(use)() { new ImportClassAlloc(); }

// M32-DAG: declare dllimport i8* @"\01??_UImportClassAlloc@@SAPAXI@Z"(i32)
// M64-DAG: declare dllimport i8* @"\01??_UImportClassAlloc@@SAPEAX_K@Z"(i64)
// G32-DAG: declare dllimport i8* @_ZN16ImportClassAllocnaEj(i32)
// G64-DAG: declare dllimport i8* @_ZN16ImportClassAllocnaEy(i64)
void UNIQ(use)() { new ImportClassAlloc[1]; }

// M32-DAG: declare dllimport void @"\01??3ImportClassAlloc@@SAXPAX@Z"(i8*)
// M64-DAG: declare dllimport void @"\01??3ImportClassAlloc@@SAXPEAX@Z"(i8*)
// G32-DAG: declare dllimport void @_ZN16ImportClassAllocdlEPv(i8*)
// G64-DAG: declare dllimport void @_ZN16ImportClassAllocdlEPv(i8*)
void UNIQ(use)(ImportClassAlloc* ptr) { delete ptr; }

// M32-DAG: declare dllimport void @"\01??_VImportClassAlloc@@SAXPAX@Z"(i8*)
// M64-DAG: declare dllimport void @"\01??_VImportClassAlloc@@SAXPEAX@Z"(i8*)
// G32-DAG: declare dllimport void @_ZN16ImportClassAllocdaEPv(i8*)
// G64-DAG: declare dllimport void @_ZN16ImportClassAllocdaEPv(i8*)
void UNIQ(use)(ImportClassAlloc* ptr) { delete[] ptr; }


// NOP-DAG_: define available_externally dllimport {{(x86_thiscallcc )?}}void @_ZN7Attribs12AlwaysInlineEv({{[^\)]*}}) #[[AlwaysInlineImportGrp:[0-9]+]]
// NOP-DAG: declare                     dllimport {{(x86_thiscallcc )?}}void @_ZN7Attribs11NeverInlineEv({{[^\)]*}})
// OPT-DAG_: define available_externally dllimport {{(x86_thiscallcc )?}}void @_ZN7Attribs12AlwaysInlineEv({{[^\)]*}}) #[[AlwaysInlineImportGrp:[0-9]+]]
// OPT-DAG: define available_externally dllimport {{(x86_thiscallcc )?}}void @_ZN7Attribs11NeverInlineEv({{[^\)]*}})
struct __declspec(dllimport) Attribs {
  __attribute__((always_inline)) void AlwaysInline() {}
  __attribute__((noinline)) void NeverInline() {}
};

#ifndef MSABI
// Import dynamic class with vtable and typeinfo.
//
// NOP-DAG: @_ZTI7Virtual = external dllimport constant {{.*}}
// NOP-DAG: define {{(x86_thiscallcc )?}}void @_ZN7Virtual20publicVirtualDefinedEv({{[^\)]*}})
// OPT-DAG_: @_ZTV7Virtual = available_externally dllimport unnamed_addr constant {{.*}}
// OPT-DAG_: @_ZTI7Virtual = external constant {{.*}}, dllimport
// OPT-DAG: define void @_ZN7Virtual20publicVirtualDefinedEv({{[^\)]*}})
// via vtable:     void @_ZN7Virtual13publicVirtualEv
// via vtable:     void @_ZN7Virtual19publicVirtualInsideEv
// via vtable:     void @_ZN7Virtual22publicVirtualInlineDefEv
// via vtable:     void @_ZN7Virtual23publicVirtualInlineDeclEv
struct __declspec(dllimport) ImportDynamicClass {
  virtual        ~ImportDynamicClass();
  virtual        void virtualDef();
  virtual        void virtualDecl();
  virtual        void virtualInclass() {}
  virtual        void virtualInlineDef();
  virtual inline void virtualInlineDecl();
};
       void ImportDynamicClass::virtualDef() {}
inline void ImportDynamicClass::virtualInlineDef() {}
       void ImportDynamicClass::virtualInlineDecl() {}

USEVIRTUAL(ImportDynamicClass)
USEVMF(ImportDynamicClass, virtualDecl)
USEVMF(ImportDynamicClass, virtualDef)
USEVMF(ImportDynamicClass, virtualInclass)
USEVMF(ImportDynamicClass, virtualInlineDef)
USEVMF(ImportDynamicClass, virtualInlineDecl)


// Import dynamic class without key function.
// Note: vtable and rtti are not imported.
//
// NOP-DAG: @_ZTI13VirtualInline = linkonce_odr unnamed_addr constant {{.*}}}{{$}}
// NOP-DAG: @_ZTS13VirtualInline = linkonce_odr constant {{.*}}"{{$}}
// NOP-DAG: declare dllimport void @_ZN13VirtualInlineD1Ev({{[^\)]*}})
//
// OPT-DAG: define available_externally dllimport void @_ZN13VirtualInlineD0Ev({{[^\)]*}})
// OPT-DAG: define available_externally dllimport void @_ZN13VirtualInlineD1Ev({{[^\)]*}})
// OPT-DAG: define available_externally dllimport void @_ZN13VirtualInlineD2Ev({{[^\)]*}})
// OPT-DAG: define available_externally dllimport void @_ZN13VirtualInline19publicVirtualInsideEv({{[^\)]*}})
struct __declspec(dllimport) VirtualInline : virtual X {
  virtual ~VirtualInline() {}
  virtual void virtualInclass() {}
};


// Import dynamic class with virtual inheritance.
//
// NOP-DAG: @_ZTI12VirtualBase1 = external dllimport constant {{.*}}
// NOP-DAG: @_ZTI12VirtualBase2 = external dllimport constant {{.*}}
// NOP-DAG: @_ZTI12VirtualBase3 = external dllimport constant {{.*}}
struct __declspec(dllimport) VirtualBase1 { virtual ~VirtualBase1(); };
struct __declspec(dllimport) VirtualBase2 : virtual VirtualBase1 { virtual ~VirtualBase2(); };
struct __declspec(dllimport) VirtualBase3 : virtual VirtualBase2 { virtual ~VirtualBase3(); };


// Import dynamic class without key function and with virtual inheritance.
// Note: vtable and rtti are not imported.
//
// NOP-DAG: @_ZTS18VirtualInlineBase1 = linkonce_odr constant {{.*}}"{{$}}
// NOP-DAG: @_ZTI18VirtualInlineBase1 = linkonce_odr unnamed_addr constant {{.*}}}{{$}}

// NOP-DAG: @_ZTS18VirtualInlineBase2 = linkonce_odr constant {{.*}}"{{$}}
// NOP-DAG: @_ZTI18VirtualInlineBase2 = linkonce_odr unnamed_addr constant {{.*}}}{{$}}

// NOP-DAG: @_ZTS18VirtualInlineBase3 = linkonce_odr constant {{.*}}"{{$}}
// NOP-DAG: @_ZTI18VirtualInlineBase3 = linkonce_odr unnamed_addr constant {{.*}}}{{$}}
struct __declspec(dllimport) VirtualInlineBase1 { virtual ~VirtualInlineBase1() {} };
struct __declspec(dllimport) VirtualInlineBase2 : virtual VirtualInlineBase1 { virtual ~VirtualInlineBase2() {} };
struct __declspec(dllimport) VirtualInlineBase3 : virtual VirtualInlineBase2 { virtual ~VirtualInlineBase3() {} };
#endif



//===----------------------------------------------------------------------===//
// Class templates
//===----------------------------------------------------------------------===//

// Imported class template.
template<typename T>
struct __declspec(dllimport) ImportClsTmpl {
                void normalDef(); // dllimport ignored
                void normalDecl();
                void normalInclass() {}
                void normalInlineDef();
         inline void normalInlineDecl();
  static        void staticDef(); // dllimport ignored
  static        void staticDecl();
  static        void staticInclass() {}
  static        void staticInlineDef();
  static inline void staticInlineDecl();

                int  Field;
  static        int  StaticField;
  static const  int  StaticConstField;
  static const  int  StaticConstFieldEqualInit = 1;
  static const  int  StaticConstFieldBraceInit{1};
  constexpr static int ConstexprField = 1;
};

template<typename T>        void ImportClsTmpl<T>::normalDef() {} // dllimport ignored
template<typename T> inline void ImportClsTmpl<T>::normalInlineDef() {}
template<typename T>        void ImportClsTmpl<T>::normalInlineDecl() {}
template<typename T>        void ImportClsTmpl<T>::staticDef() {} // dllimport ignored
template<typename T> inline void ImportClsTmpl<T>::staticInlineDef() {}
template<typename T>        void ImportClsTmpl<T>::staticInlineDecl() {}


// Non-exported class template.
template<typename T>
struct ClassTmpl {
                void normalDef();
                void normalDecl();
                void normalInclass() {}
                void normalInlineDef();
         inline void normalInlineDecl();
  static        void staticDef();
  static        void staticDecl();
  static        void staticInclass() {}
  static        void staticInlineDef();
  static inline void staticInlineDecl();

                int  Field;
  static        int  StaticField;
  static const  int  StaticConstField;
  static const  int  StaticConstFieldEqualInit = 1;
  static const  int  StaticConstFieldBraceInit{1};
  constexpr static int ConstexprField = 1;
};

template<typename T>        void ClassTmpl<T>::normalDef() {}
template<typename T> inline void ClassTmpl<T>::normalInlineDef() {}
template<typename T>        void ClassTmpl<T>::normalInlineDecl() {}
template<typename T>        void ClassTmpl<T>::staticDef() {}
template<typename T> inline void ClassTmpl<T>::staticInlineDef() {}
template<typename T>        void ClassTmpl<T>::staticInlineDecl() {}

template<typename T>        int  ClassTmpl<T>::StaticField = 1;
template<typename T> const  int  ClassTmpl<T>::StaticConstField = 1;
template<typename T> const  int  ClassTmpl<T>::StaticConstFieldEqualInit;
template<typename T> const  int  ClassTmpl<T>::StaticConstFieldBraceInit;
template<typename T> constexpr int ClassTmpl<T>::ConstexprField;


// Import explicit instantiation declaration of an imported class template.
//
// FIXME: normalDef should not be imported.
// M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalDef@?$ImportClsTmpl@UExplicitDecl_Imported@@@@QAEXXZ"(
// M64-DAG: declare dllimport                void @"\01?normalDef@?$ImportClsTmpl@UExplicitDecl_Imported@@@@QEAAXXZ"(
// M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalInclass@?$ImportClsTmpl@UExplicitDecl_Imported@@@@QAEXXZ"(
// M64-DAG: declare dllimport                void @"\01?normalInclass@?$ImportClsTmpl@UExplicitDecl_Imported@@@@QEAAXXZ"(
// M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalInlineDef@?$ImportClsTmpl@UExplicitDecl_Imported@@@@QAEXXZ"(
// M64-DAG: declare dllimport                void @"\01?normalInlineDef@?$ImportClsTmpl@UExplicitDecl_Imported@@@@QEAAXXZ"(
// M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalInlineDecl@?$ImportClsTmpl@UExplicitDecl_Imported@@@@QAEXXZ"(
// M64-DAG: declare dllimport                void @"\01?normalInlineDecl@?$ImportClsTmpl@UExplicitDecl_Imported@@@@QEAAXXZ"(
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN13ImportClsTmplI21ExplicitDecl_ImportedE9normalDefEv(
// G64-DAG: declare dllimport                void @_ZN13ImportClsTmplI21ExplicitDecl_ImportedE9normalDefEv(
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN13ImportClsTmplI21ExplicitDecl_ImportedE13normalInclassEv(
// G64-DAG: declare dllimport                void @_ZN13ImportClsTmplI21ExplicitDecl_ImportedE13normalInclassEv(
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN13ImportClsTmplI21ExplicitDecl_ImportedE15normalInlineDefEv(
// G64-DAG: declare dllimport                void @_ZN13ImportClsTmplI21ExplicitDecl_ImportedE15normalInlineDefEv(
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN13ImportClsTmplI21ExplicitDecl_ImportedE16normalInlineDeclEv(
// G64-DAG: declare dllimport                void @_ZN13ImportClsTmplI21ExplicitDecl_ImportedE16normalInlineDeclEv(
//
// FIXME: staticDef should not be imported.
// MSC-DAG: declare dllimport                void @"\01?staticDef@?$ImportClsTmpl@UExplicitDecl_Imported@@@@SAXXZ"()
// MSC-DAG: declare dllimport                void @"\01?staticInclass@?$ImportClsTmpl@UExplicitDecl_Imported@@@@SAXXZ"()
// MSC-DAG: declare dllimport                void @"\01?staticInlineDef@?$ImportClsTmpl@UExplicitDecl_Imported@@@@SAXXZ"()
// MSC-DAG: declare dllimport                void @"\01?staticInlineDecl@?$ImportClsTmpl@UExplicitDecl_Imported@@@@SAXXZ"()
// GNU-DAG: declare dllimport                void @_ZN13ImportClsTmplI21ExplicitDecl_ImportedE9staticDefEv()
// GNU-DAG: declare dllimport                void @_ZN13ImportClsTmplI21ExplicitDecl_ImportedE13staticInclassEv()
// GNU-DAG: declare dllimport                void @_ZN13ImportClsTmplI21ExplicitDecl_ImportedE15staticInlineDefEv()
// GNU-DAG: declare dllimport                void @_ZN13ImportClsTmplI21ExplicitDecl_ImportedE16staticInlineDeclEv()
//
// MSC-DAG: @"\01?StaticField@?$ImportClsTmpl@UExplicitDecl_Imported@@@@2HA"               = external dllimport global i32
// MSC-DAG: @"\01?StaticConstField@?$ImportClsTmpl@UExplicitDecl_Imported@@@@2HB"          = external dllimport constant i32
// MSC-DAG: @"\01?StaticConstFieldEqualInit@?$ImportClsTmpl@UExplicitDecl_Imported@@@@2HB" = available_externally dllimport constant i32 1, align 4
// MSC-DAG: @"\01?StaticConstFieldBraceInit@?$ImportClsTmpl@UExplicitDecl_Imported@@@@2HB" = available_externally dllimport constant i32 1, align 4
// MSC-DAG: @"\01?ConstexprField@?$ImportClsTmpl@UExplicitDecl_Imported@@@@2HB"            = available_externally dllimport constant i32 1, align 4
// GNU-DAG: @_ZN13ImportClsTmplI21ExplicitDecl_ImportedE11StaticFieldE                     = external dllimport global i32
// GNU-DAG: @_ZN13ImportClsTmplI21ExplicitDecl_ImportedE16StaticConstFieldE                = external dllimport constant i32
// GNU-DAG: @_ZN13ImportClsTmplI21ExplicitDecl_ImportedE25StaticConstFieldEqualInitE       = external dllimport constant i32
// GNU-DAG: @_ZN13ImportClsTmplI21ExplicitDecl_ImportedE25StaticConstFieldBraceInitE       = external dllimport constant i32
// GNU-DAG: @_ZN13ImportClsTmplI21ExplicitDecl_ImportedE14ConstexprFieldE                  = external dllimport constant i32
extern template struct ImportClsTmpl<ExplicitDecl_Imported>;

USEMF(ImportClsTmpl<ExplicitDecl_Imported>, normalDef)
USEMF(ImportClsTmpl<ExplicitDecl_Imported>, normalDecl)
USEMF(ImportClsTmpl<ExplicitDecl_Imported>, normalInclass)
USEMF(ImportClsTmpl<ExplicitDecl_Imported>, normalInlineDef)
USEMF(ImportClsTmpl<ExplicitDecl_Imported>, normalInlineDecl)
USEMF(ImportClsTmpl<ExplicitDecl_Imported>, staticDef)
USEMF(ImportClsTmpl<ExplicitDecl_Imported>, staticDecl)
USEMF(ImportClsTmpl<ExplicitDecl_Imported>, staticInclass)
USEMF(ImportClsTmpl<ExplicitDecl_Imported>, staticInlineDef)
USEMF(ImportClsTmpl<ExplicitDecl_Imported>, staticInlineDecl)

USEMV(ImportClsTmpl<ExplicitDecl_Imported>, StaticField)
USEMV(ImportClsTmpl<ExplicitDecl_Imported>, StaticConstField)
USEMV(ImportClsTmpl<ExplicitDecl_Imported>, StaticConstFieldEqualInit)
USEMV(ImportClsTmpl<ExplicitDecl_Imported>, StaticConstFieldBraceInit)
USEMV(ImportClsTmpl<ExplicitDecl_Imported>, ConstexprField)


// Import explicit instantiation definition of an imported class template.
//
// FIXME: normalDef should not be imported.
// M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalDef@?$ImportClsTmpl@UExplicitInst_Imported@@@@QAEXXZ"(
// M64-DAG: declare dllimport                void @"\01?normalDef@?$ImportClsTmpl@UExplicitInst_Imported@@@@QEAAXXZ"(
// M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalInclass@?$ImportClsTmpl@UExplicitInst_Imported@@@@QAEXXZ"(
// M64-DAG: declare dllimport                void @"\01?normalInclass@?$ImportClsTmpl@UExplicitInst_Imported@@@@QEAAXXZ"(
// M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalInlineDef@?$ImportClsTmpl@UExplicitInst_Imported@@@@QAEXXZ"(
// M64-DAG: declare dllimport                void @"\01?normalInlineDef@?$ImportClsTmpl@UExplicitInst_Imported@@@@QEAAXXZ"(
// M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalInlineDecl@?$ImportClsTmpl@UExplicitInst_Imported@@@@QAEXXZ"(
// M64-DAG: declare dllimport                void @"\01?normalInlineDecl@?$ImportClsTmpl@UExplicitInst_Imported@@@@QEAAXXZ"(
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN13ImportClsTmplI21ExplicitInst_ImportedE9normalDefEv(
// G64-DAG: declare dllimport                void @_ZN13ImportClsTmplI21ExplicitInst_ImportedE9normalDefEv(
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN13ImportClsTmplI21ExplicitInst_ImportedE13normalInclassEv(
// G64-DAG: declare dllimport                void @_ZN13ImportClsTmplI21ExplicitInst_ImportedE13normalInclassEv(
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN13ImportClsTmplI21ExplicitInst_ImportedE15normalInlineDefEv(
// G64-DAG: declare dllimport                void @_ZN13ImportClsTmplI21ExplicitInst_ImportedE15normalInlineDefEv(
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN13ImportClsTmplI21ExplicitInst_ImportedE16normalInlineDeclEv(
// G64-DAG: declare dllimport                void @_ZN13ImportClsTmplI21ExplicitInst_ImportedE16normalInlineDeclEv(
//
// FIXME: staticDef should not be imported.
// MSC-DAG: declare dllimport                void @"\01?staticDef@?$ImportClsTmpl@UExplicitInst_Imported@@@@SAXXZ"()
// MSC-DAG: declare dllimport                void @"\01?staticInclass@?$ImportClsTmpl@UExplicitInst_Imported@@@@SAXXZ"()
// MSC-DAG: declare dllimport                void @"\01?staticInlineDef@?$ImportClsTmpl@UExplicitInst_Imported@@@@SAXXZ"()
// MSC-DAG: declare dllimport                void @"\01?staticInlineDecl@?$ImportClsTmpl@UExplicitInst_Imported@@@@SAXXZ"()
// GNU-DAG: declare dllimport                void @_ZN13ImportClsTmplI21ExplicitInst_ImportedE9staticDefEv()
// GNU-DAG: declare dllimport                void @_ZN13ImportClsTmplI21ExplicitInst_ImportedE13staticInclassEv()
// GNU-DAG: declare dllimport                void @_ZN13ImportClsTmplI21ExplicitInst_ImportedE15staticInlineDefEv()
// GNU-DAG: declare dllimport                void @_ZN13ImportClsTmplI21ExplicitInst_ImportedE16staticInlineDeclEv()
//
// MSC-DAG: @"\01?StaticField@?$ImportClsTmpl@UExplicitInst_Imported@@@@2HA"               = external dllimport global i32
// MSC-DAG: @"\01?StaticConstField@?$ImportClsTmpl@UExplicitInst_Imported@@@@2HB"          = external dllimport constant i32
// MSC-DAG: @"\01?StaticConstFieldEqualInit@?$ImportClsTmpl@UExplicitInst_Imported@@@@2HB" = available_externally dllimport constant i32 1, align 4
// MSC-DAG: @"\01?StaticConstFieldBraceInit@?$ImportClsTmpl@UExplicitInst_Imported@@@@2HB" = available_externally dllimport constant i32 1, align 4
// MSC-DAG: @"\01?ConstexprField@?$ImportClsTmpl@UExplicitInst_Imported@@@@2HB"            = available_externally dllimport constant i32 1, align 4
// GNU-DAG: @_ZN13ImportClsTmplI21ExplicitInst_ImportedE11StaticFieldE                     = external dllimport global i32
// GNU-DAG: @_ZN13ImportClsTmplI21ExplicitInst_ImportedE16StaticConstFieldE                = external dllimport constant i32
// GNU-DAG: @_ZN13ImportClsTmplI21ExplicitInst_ImportedE25StaticConstFieldEqualInitE       = external dllimport constant i32
// GNU-DAG: @_ZN13ImportClsTmplI21ExplicitInst_ImportedE25StaticConstFieldBraceInitE       = external dllimport constant i32
// GNU-DAG: @_ZN13ImportClsTmplI21ExplicitInst_ImportedE14ConstexprFieldE                  = external dllimport constant i32
template struct ImportClsTmpl<ExplicitInst_Imported>;

USEMF(ImportClsTmpl<ExplicitInst_Imported>, normalDef)
USEMF(ImportClsTmpl<ExplicitInst_Imported>, normalDecl)
USEMF(ImportClsTmpl<ExplicitInst_Imported>, normalInclass)
USEMF(ImportClsTmpl<ExplicitInst_Imported>, normalInlineDef)
USEMF(ImportClsTmpl<ExplicitInst_Imported>, normalInlineDecl)
USEMF(ImportClsTmpl<ExplicitInst_Imported>, staticDef)
USEMF(ImportClsTmpl<ExplicitInst_Imported>, staticDecl)
USEMF(ImportClsTmpl<ExplicitInst_Imported>, staticInclass)
USEMF(ImportClsTmpl<ExplicitInst_Imported>, staticInlineDef)
USEMF(ImportClsTmpl<ExplicitInst_Imported>, staticInlineDecl)

USEMV(ImportClsTmpl<ExplicitInst_Imported>, StaticField)
USEMV(ImportClsTmpl<ExplicitInst_Imported>, StaticConstField)
USEMV(ImportClsTmpl<ExplicitInst_Imported>, StaticConstFieldEqualInit)
USEMV(ImportClsTmpl<ExplicitInst_Imported>, StaticConstFieldBraceInit)
USEMV(ImportClsTmpl<ExplicitInst_Imported>, ConstexprField)


// Import specialization of an imported class template.
template<>
struct __declspec(dllimport) ImportClsTmpl<ExplicitSpec_Imported> {
  // M32-DAG: define            x86_thiscallcc void @"\01?normalDef@?$ImportClsTmpl@UExplicitSpec_Imported@@@@QAEXXZ"(
  // M64-DAG: define                           void @"\01?normalDef@?$ImportClsTmpl@UExplicitSpec_Imported@@@@QEAAXXZ"(
  // M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalDecl@?$ImportClsTmpl@UExplicitSpec_Imported@@@@QAEXXZ"(
  // M64-DAG: declare dllimport                void @"\01?normalDecl@?$ImportClsTmpl@UExplicitSpec_Imported@@@@QEAAXXZ"(
  // M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalInclass@?$ImportClsTmpl@UExplicitSpec_Imported@@@@QAEXXZ"(
  // M64-DAG: declare dllimport                void @"\01?normalInclass@?$ImportClsTmpl@UExplicitSpec_Imported@@@@QEAAXXZ"(
  // M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalInlineDef@?$ImportClsTmpl@UExplicitSpec_Imported@@@@QAEXXZ"(
  // M64-DAG: declare dllimport                void @"\01?normalInlineDef@?$ImportClsTmpl@UExplicitSpec_Imported@@@@QEAAXXZ"(
  // M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalInlineDecl@?$ImportClsTmpl@UExplicitSpec_Imported@@@@QAEXXZ"(
  // M64-DAG: declare dllimport                void @"\01?normalInlineDecl@?$ImportClsTmpl@UExplicitSpec_Imported@@@@QEAAXXZ"(
  // G32-DAG: define            x86_thiscallcc void @_ZN13ImportClsTmplI21ExplicitSpec_ImportedE9normalDefEv(
  // G64-DAG: define                           void @_ZN13ImportClsTmplI21ExplicitSpec_ImportedE9normalDefEv(
  // G32-DAG: declare dllimport x86_thiscallcc void @_ZN13ImportClsTmplI21ExplicitSpec_ImportedE10normalDeclEv(
  // G64-DAG: declare dllimport                void @_ZN13ImportClsTmplI21ExplicitSpec_ImportedE10normalDeclEv(
  // G32-DAG: declare dllimport x86_thiscallcc void @_ZN13ImportClsTmplI21ExplicitSpec_ImportedE13normalInclassEv(
  // G64-DAG: declare dllimport                void @_ZN13ImportClsTmplI21ExplicitSpec_ImportedE13normalInclassEv(
  // G32-DAG: declare dllimport x86_thiscallcc void @_ZN13ImportClsTmplI21ExplicitSpec_ImportedE15normalInlineDefEv(
  // G64-DAG: declare dllimport                void @_ZN13ImportClsTmplI21ExplicitSpec_ImportedE15normalInlineDefEv(
  // G32-DAG: declare dllimport x86_thiscallcc void @_ZN13ImportClsTmplI21ExplicitSpec_ImportedE16normalInlineDeclEv(
  // G64-DAG: declare dllimport                void @_ZN13ImportClsTmplI21ExplicitSpec_ImportedE16normalInlineDeclEv(
                void normalDef(); // dllimport ignored
                void normalDecl();
                void normalInclass() {}
                void normalInlineDef();
         inline void normalInlineDecl();

  // MSC-DAG: define                           void @"\01?staticDef@?$ImportClsTmpl@UExplicitSpec_Imported@@@@SAXXZ"()
  // MSC-DAG: declare dllimport                void @"\01?staticDecl@?$ImportClsTmpl@UExplicitSpec_Imported@@@@SAXXZ"()
  // MSC-DAG: declare dllimport                void @"\01?staticInclass@?$ImportClsTmpl@UExplicitSpec_Imported@@@@SAXXZ"()
  // MSC-DAG: declare dllimport                void @"\01?staticInlineDef@?$ImportClsTmpl@UExplicitSpec_Imported@@@@SAXXZ"()
  // MSC-DAG: declare dllimport                void @"\01?staticInlineDecl@?$ImportClsTmpl@UExplicitSpec_Imported@@@@SAXXZ"()
  // GNU-DAG: define                           void @_ZN13ImportClsTmplI21ExplicitSpec_ImportedE9staticDefEv()
  // GNU-DAG: declare dllimport                void @_ZN13ImportClsTmplI21ExplicitSpec_ImportedE10staticDeclEv()
  // GNU-DAG: declare dllimport                void @_ZN13ImportClsTmplI21ExplicitSpec_ImportedE13staticInclassEv()
  // GNU-DAG: declare dllimport                void @_ZN13ImportClsTmplI21ExplicitSpec_ImportedE15staticInlineDefEv()
  // GNU-DAG: declare dllimport                void @_ZN13ImportClsTmplI21ExplicitSpec_ImportedE16staticInlineDeclEv()
  static        void staticDef(); // dllimport ignored
  static        void staticDecl();
  static        void staticInclass() {}
  static        void staticInlineDef();
  static inline void staticInlineDecl();

  // MSC-DAG: @"\01?StaticField@?$ImportClsTmpl@UExplicitSpec_Imported@@@@2HA"               = external dllimport global i32
  // MSC-DAG: @"\01?StaticConstField@?$ImportClsTmpl@UExplicitSpec_Imported@@@@2HB"          = external dllimport constant i32
  // MSC-DAG: @"\01?StaticConstFieldEqualInit@?$ImportClsTmpl@UExplicitSpec_Imported@@@@2HB" = available_externally dllimport constant i32 1, align 4
  // MSC-DAG: @"\01?StaticConstFieldBraceInit@?$ImportClsTmpl@UExplicitSpec_Imported@@@@2HB" = available_externally dllimport constant i32 1, align 4
  // MSC-DAG: @"\01?ConstexprField@?$ImportClsTmpl@UExplicitSpec_Imported@@@@2HB"            = available_externally dllimport constant i32 1, align 4
  // GNU-DAG: @_ZN13ImportClsTmplI21ExplicitSpec_ImportedE11StaticFieldE                     = external dllimport global i32
  // GNU-DAG: @_ZN13ImportClsTmplI21ExplicitSpec_ImportedE16StaticConstFieldE                = external dllimport constant i32
  // GNU-DAG: @_ZN13ImportClsTmplI21ExplicitSpec_ImportedE25StaticConstFieldEqualInitE       = external dllimport constant i32
  // GNU-DAG: @_ZN13ImportClsTmplI21ExplicitSpec_ImportedE25StaticConstFieldBraceInitE       = external dllimport constant i32
  // GNU-DAG: @_ZN13ImportClsTmplI21ExplicitSpec_ImportedE14ConstexprFieldE                  = external dllimport constant i32
                int  Field;
  static        int  StaticField;
  static const  int  StaticConstField;
  static const  int  StaticConstFieldEqualInit = 1;
  static const  int  StaticConstFieldBraceInit{1};
  constexpr static int ConstexprField = 1;
};

void ImportClsTmpl<ExplicitSpec_Imported>::normalDef() {} // dllimport ignored
void ImportClsTmpl<ExplicitSpec_Imported>::staticDef() {} // dllimport ignored

USEMF(ImportClsTmpl<ExplicitSpec_Imported>, normalDef)
USEMF(ImportClsTmpl<ExplicitSpec_Imported>, normalDecl)
USEMF(ImportClsTmpl<ExplicitSpec_Imported>, normalInclass)
USEMF(ImportClsTmpl<ExplicitSpec_Imported>, normalInlineDef)
USEMF(ImportClsTmpl<ExplicitSpec_Imported>, normalInlineDecl)
USEMF(ImportClsTmpl<ExplicitSpec_Imported>, staticDef)
USEMF(ImportClsTmpl<ExplicitSpec_Imported>, staticDecl)
USEMF(ImportClsTmpl<ExplicitSpec_Imported>, staticInclass)
USEMF(ImportClsTmpl<ExplicitSpec_Imported>, staticInlineDef)
USEMF(ImportClsTmpl<ExplicitSpec_Imported>, staticInlineDecl)

USEMV(ImportClsTmpl<ExplicitSpec_Imported>, StaticField)
USEMV(ImportClsTmpl<ExplicitSpec_Imported>, StaticConstField)
USEMV(ImportClsTmpl<ExplicitSpec_Imported>, StaticConstFieldEqualInit)
USEMV(ImportClsTmpl<ExplicitSpec_Imported>, StaticConstFieldBraceInit)
USEMV(ImportClsTmpl<ExplicitSpec_Imported>, ConstexprField)



// Not importing specialization of an imported class template without explicit
// dllimport.
// M32-DAG: define x86_thiscallcc void @"\01?normalDef@?$ImportClsTmpl@UExplicitSpec_NotImported@@@@QAEXXZ"(
// M64-DAG: define                void @"\01?normalDef@?$ImportClsTmpl@UExplicitSpec_NotImported@@@@QEAAXXZ"(
// G32-DAG: define x86_thiscallcc void @_ZN13ImportClsTmplI24ExplicitSpec_NotImportedE9normalDefEv(
// G64-DAG: define                void @_ZN13ImportClsTmplI24ExplicitSpec_NotImportedE9normalDefEv(
template<>
struct ImportClsTmpl<ExplicitSpec_NotImported> {
  void normalDef();
};
void ImportClsTmpl<ExplicitSpec_NotImported>::normalDef() {}


// Import explicit instantiation declaration of a non-imported class template.
//
// M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalDef@?$ClassTmpl@UExplicitDecl_Imported@@@@QAEXXZ"(
// M64-DAG: declare dllimport                void @"\01?normalDef@?$ClassTmpl@UExplicitDecl_Imported@@@@QEAAXXZ"(
// M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalDecl@?$ClassTmpl@UExplicitDecl_Imported@@@@QAEXXZ"(
// M64-DAG: declare dllimport                void @"\01?normalDecl@?$ClassTmpl@UExplicitDecl_Imported@@@@QEAAXXZ"(
// M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalInclass@?$ClassTmpl@UExplicitDecl_Imported@@@@QAEXXZ"(
// M64-DAG: declare dllimport                void @"\01?normalInclass@?$ClassTmpl@UExplicitDecl_Imported@@@@QEAAXXZ"(
// M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalInlineDef@?$ClassTmpl@UExplicitDecl_Imported@@@@QAEXXZ"(
// M64-DAG: declare dllimport                void @"\01?normalInlineDef@?$ClassTmpl@UExplicitDecl_Imported@@@@QEAAXXZ"(
// M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalInlineDecl@?$ClassTmpl@UExplicitDecl_Imported@@@@QAEXXZ"(
// M64-DAG: declare dllimport                void @"\01?normalInlineDecl@?$ClassTmpl@UExplicitDecl_Imported@@@@QEAAXXZ"(
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN9ClassTmplI21ExplicitDecl_ImportedE9normalDefEv(
// G64-DAG: declare dllimport                void @_ZN9ClassTmplI21ExplicitDecl_ImportedE9normalDefEv(
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN9ClassTmplI21ExplicitDecl_ImportedE10normalDeclEv(
// G64-DAG: declare dllimport                void @_ZN9ClassTmplI21ExplicitDecl_ImportedE10normalDeclEv(
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN9ClassTmplI21ExplicitDecl_ImportedE13normalInclassEv(
// G64-DAG: declare dllimport                void @_ZN9ClassTmplI21ExplicitDecl_ImportedE13normalInclassEv(
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN9ClassTmplI21ExplicitDecl_ImportedE15normalInlineDefEv(
// G64-DAG: declare dllimport                void @_ZN9ClassTmplI21ExplicitDecl_ImportedE15normalInlineDefEv(
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN9ClassTmplI21ExplicitDecl_ImportedE16normalInlineDeclEv(
// G64-DAG: declare dllimport                void @_ZN9ClassTmplI21ExplicitDecl_ImportedE16normalInlineDeclEv(
//
// MSC-DAG: declare dllimport                void @"\01?staticDef@?$ClassTmpl@UExplicitDecl_Imported@@@@SAXXZ"()
// MSC-DAG: declare dllimport                void @"\01?staticDecl@?$ClassTmpl@UExplicitDecl_Imported@@@@SAXXZ"()
// MSC-DAG: declare dllimport                void @"\01?staticInclass@?$ClassTmpl@UExplicitDecl_Imported@@@@SAXXZ"()
// MSC-DAG: declare dllimport                void @"\01?staticInlineDef@?$ClassTmpl@UExplicitDecl_Imported@@@@SAXXZ"()
// MSC-DAG: declare dllimport                void @"\01?staticInlineDecl@?$ClassTmpl@UExplicitDecl_Imported@@@@SAXXZ"()
// GNU-DAG: declare dllimport                void @_ZN9ClassTmplI21ExplicitDecl_ImportedE9staticDefEv()
// GNU-DAG: declare dllimport                void @_ZN9ClassTmplI21ExplicitDecl_ImportedE10staticDeclEv()
// GNU-DAG: declare dllimport                void @_ZN9ClassTmplI21ExplicitDecl_ImportedE13staticInclassEv()
// GNU-DAG: declare dllimport                void @_ZN9ClassTmplI21ExplicitDecl_ImportedE15staticInlineDefEv()
// GNU-DAG: declare dllimport                void @_ZN9ClassTmplI21ExplicitDecl_ImportedE16staticInlineDeclEv()
//
// MSC-DAG: @"\01?StaticField@?$ClassTmpl@UExplicitDecl_Imported@@@@2HA"               = external dllimport global i32
// MSC-DAG: @"\01?StaticConstField@?$ClassTmpl@UExplicitDecl_Imported@@@@2HB"          = external dllimport constant i32
// MSC-DAG: @"\01?StaticConstFieldEqualInit@?$ClassTmpl@UExplicitDecl_Imported@@@@2HB" = available_externally dllimport constant i32 1, align 4
// MSC-DAG: @"\01?StaticConstFieldBraceInit@?$ClassTmpl@UExplicitDecl_Imported@@@@2HB" = available_externally dllimport constant i32 1, align 4
// MSC-DAG: @"\01?ConstexprField@?$ClassTmpl@UExplicitDecl_Imported@@@@2HB"            = available_externally dllimport constant i32 1, align 4
// GNU-DAG: @_ZN9ClassTmplI21ExplicitDecl_ImportedE11StaticFieldE                      = external dllimport global i32
// GNU-DAG: @_ZN9ClassTmplI21ExplicitDecl_ImportedE16StaticConstFieldE                 = external dllimport constant i32
// GNU-DAG: @_ZN9ClassTmplI21ExplicitDecl_ImportedE25StaticConstFieldEqualInitE        = external dllimport constant i32
// GNU-DAG: @_ZN9ClassTmplI21ExplicitDecl_ImportedE25StaticConstFieldBraceInitE        = external dllimport constant i32
// GNU-DAG: @_ZN9ClassTmplI21ExplicitDecl_ImportedE14ConstexprFieldE                   = external dllimport constant i32
extern template struct __declspec(dllimport) ClassTmpl<ExplicitDecl_Imported>;

USEMF(ClassTmpl<ExplicitDecl_Imported>, normalDef)
USEMF(ClassTmpl<ExplicitDecl_Imported>, normalDecl)
USEMF(ClassTmpl<ExplicitDecl_Imported>, normalInclass)
USEMF(ClassTmpl<ExplicitDecl_Imported>, normalInlineDef)
USEMF(ClassTmpl<ExplicitDecl_Imported>, normalInlineDecl)
USEMF(ClassTmpl<ExplicitDecl_Imported>, staticDef)
USEMF(ClassTmpl<ExplicitDecl_Imported>, staticDecl)
USEMF(ClassTmpl<ExplicitDecl_Imported>, staticInclass)
USEMF(ClassTmpl<ExplicitDecl_Imported>, staticInlineDef)
USEMF(ClassTmpl<ExplicitDecl_Imported>, staticInlineDecl)

USEMV(ClassTmpl<ExplicitDecl_Imported>, StaticField)
USEMV(ClassTmpl<ExplicitDecl_Imported>, StaticConstField)
USEMV(ClassTmpl<ExplicitDecl_Imported>, StaticConstFieldEqualInit)
USEMV(ClassTmpl<ExplicitDecl_Imported>, StaticConstFieldBraceInit)
USEMV(ClassTmpl<ExplicitDecl_Imported>, ConstexprField)


// Export explicit instantiation definition of a non-exported class template.
//
// M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalDef@?$ClassTmpl@UExplicitInst_Imported@@@@QAEXXZ"(
// M64-DAG: declare dllimport                void @"\01?normalDef@?$ClassTmpl@UExplicitInst_Imported@@@@QEAAXXZ"(
// M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalDecl@?$ClassTmpl@UExplicitInst_Imported@@@@QAEXXZ"(
// M64-DAG: declare dllimport                void @"\01?normalDecl@?$ClassTmpl@UExplicitInst_Imported@@@@QEAAXXZ"(
// M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalInclass@?$ClassTmpl@UExplicitInst_Imported@@@@QAEXXZ"(
// M64-DAG: declare dllimport                void @"\01?normalInclass@?$ClassTmpl@UExplicitInst_Imported@@@@QEAAXXZ"(
// M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalInlineDef@?$ClassTmpl@UExplicitInst_Imported@@@@QAEXXZ"(
// M64-DAG: declare dllimport                void @"\01?normalInlineDef@?$ClassTmpl@UExplicitInst_Imported@@@@QEAAXXZ"(
// M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalInlineDecl@?$ClassTmpl@UExplicitInst_Imported@@@@QAEXXZ"(
// M64-DAG: declare dllimport                void @"\01?normalInlineDecl@?$ClassTmpl@UExplicitInst_Imported@@@@QEAAXXZ"(
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN9ClassTmplI21ExplicitInst_ImportedE9normalDefEv(
// G64-DAG: declare dllimport                void @_ZN9ClassTmplI21ExplicitInst_ImportedE9normalDefEv(
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN9ClassTmplI21ExplicitInst_ImportedE10normalDeclEv(
// G64-DAG: declare dllimport                void @_ZN9ClassTmplI21ExplicitInst_ImportedE10normalDeclEv(
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN9ClassTmplI21ExplicitInst_ImportedE13normalInclassEv(
// G64-DAG: declare dllimport                void @_ZN9ClassTmplI21ExplicitInst_ImportedE13normalInclassEv(
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN9ClassTmplI21ExplicitInst_ImportedE15normalInlineDefEv(
// G64-DAG: declare dllimport                void @_ZN9ClassTmplI21ExplicitInst_ImportedE15normalInlineDefEv(
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN9ClassTmplI21ExplicitInst_ImportedE16normalInlineDeclEv(
// G64-DAG: declare dllimport                void @_ZN9ClassTmplI21ExplicitInst_ImportedE16normalInlineDeclEv(
//
// MSC-DAG: declare dllimport                void @"\01?staticDef@?$ClassTmpl@UExplicitInst_Imported@@@@SAXXZ"()
// MSC-DAG: declare dllimport                void @"\01?staticDecl@?$ClassTmpl@UExplicitInst_Imported@@@@SAXXZ"()
// MSC-DAG: declare dllimport                void @"\01?staticInclass@?$ClassTmpl@UExplicitInst_Imported@@@@SAXXZ"()
// MSC-DAG: declare dllimport                void @"\01?staticInlineDef@?$ClassTmpl@UExplicitInst_Imported@@@@SAXXZ"()
// MSC-DAG: declare dllimport                void @"\01?staticInlineDecl@?$ClassTmpl@UExplicitInst_Imported@@@@SAXXZ"()
// GNU-DAG: declare dllimport                void @_ZN9ClassTmplI21ExplicitInst_ImportedE9staticDefEv()
// GNU-DAG: declare dllimport                void @_ZN9ClassTmplI21ExplicitInst_ImportedE10staticDeclEv()
// GNU-DAG: declare dllimport                void @_ZN9ClassTmplI21ExplicitInst_ImportedE13staticInclassEv()
// GNU-DAG: declare dllimport                void @_ZN9ClassTmplI21ExplicitInst_ImportedE15staticInlineDefEv()
// GNU-DAG: declare dllimport                void @_ZN9ClassTmplI21ExplicitInst_ImportedE16staticInlineDeclEv()
//
// MSC-DAG: @"\01?StaticField@?$ClassTmpl@UExplicitInst_Imported@@@@2HA"               = available_externally dllimport global i32 1, align 4
// MSC-DAG: @"\01?StaticConstField@?$ClassTmpl@UExplicitInst_Imported@@@@2HB"          = available_externally dllimport constant i32 1, align 4
// MSC-DAG: @"\01?StaticConstFieldEqualInit@?$ClassTmpl@UExplicitInst_Imported@@@@2HB" = available_externally dllimport constant i32 1, align 4
// MSC-DAG: @"\01?StaticConstFieldBraceInit@?$ClassTmpl@UExplicitInst_Imported@@@@2HB" = available_externally dllimport constant i32 1, align 4
// MSC-DAG: @"\01?ConstexprField@?$ClassTmpl@UExplicitInst_Imported@@@@2HB"            = available_externally dllimport constant i32 1, align 4
// GNU-DAG: @_ZN9ClassTmplI21ExplicitInst_ImportedE11StaticFieldE                      = available_externally dllimport global i32 1, align 4
// GNU-DAG: @_ZN9ClassTmplI21ExplicitInst_ImportedE16StaticConstFieldE                 = available_externally dllimport constant i32 1, align 4
// GNU-DAG: @_ZN9ClassTmplI21ExplicitInst_ImportedE25StaticConstFieldEqualInitE        = available_externally dllimport constant i32 1, align 4
// GNU-DAG: @_ZN9ClassTmplI21ExplicitInst_ImportedE25StaticConstFieldBraceInitE        = available_externally dllimport constant i32 1, align 4
// GNU-DAG: @_ZN9ClassTmplI21ExplicitInst_ImportedE14ConstexprFieldE                   = available_externally dllimport constant i32 1, align 4
template struct __declspec(dllimport) ClassTmpl<ExplicitInst_Imported>;

USEMF(ClassTmpl<ExplicitInst_Imported>, normalDef)
USEMF(ClassTmpl<ExplicitInst_Imported>, normalDecl)
USEMF(ClassTmpl<ExplicitInst_Imported>, normalInclass)
USEMF(ClassTmpl<ExplicitInst_Imported>, normalInlineDef)
USEMF(ClassTmpl<ExplicitInst_Imported>, normalInlineDecl)
USEMF(ClassTmpl<ExplicitInst_Imported>, staticDef)
USEMF(ClassTmpl<ExplicitInst_Imported>, staticDecl)
USEMF(ClassTmpl<ExplicitInst_Imported>, staticInclass)
USEMF(ClassTmpl<ExplicitInst_Imported>, staticInlineDef)
USEMF(ClassTmpl<ExplicitInst_Imported>, staticInlineDecl)

USEMV(ClassTmpl<ExplicitInst_Imported>, StaticField)
USEMV(ClassTmpl<ExplicitInst_Imported>, StaticConstField)
USEMV(ClassTmpl<ExplicitInst_Imported>, StaticConstFieldEqualInit)
USEMV(ClassTmpl<ExplicitInst_Imported>, StaticConstFieldBraceInit)
USEMV(ClassTmpl<ExplicitInst_Imported>, ConstexprField)


// Import specialization of a non-imported class template.
// M32-DAG: declare dllimport x86_thiscallcc void @"\01?normalDecl@?$ClassTmpl@UExplicitSpec_Imported@@@@QAEXXZ"(
// M64-DAG: declare dllimport                void @"\01?normalDecl@?$ClassTmpl@UExplicitSpec_Imported@@@@QEAAXXZ"(
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN9ClassTmplI21ExplicitSpec_ImportedE10normalDeclEv(
// G64-DAG: declare dllimport                void @_ZN9ClassTmplI21ExplicitSpec_ImportedE10normalDeclEv(
template<>
struct __declspec(dllimport) ClassTmpl<ExplicitSpec_Imported>
{
  void normalDecl();
};

USEMF(ClassTmpl<ExplicitSpec_Imported>, normalDecl)
