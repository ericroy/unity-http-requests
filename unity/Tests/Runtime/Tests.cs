using System.Collections;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using NUnit.Framework;

using UnityHttpRequests;

namespace Tests
{

    [TestFixture]
    public class TestSuite
    {
        private HttpSession session;

        [SetUp]
        public void Setup()
        {
            session = new HttpSession();
        }

        [TearDown]
        public void TearDown()
        {
            if (session != null)
            {
                session.Dispose();
            }
        }

        [Test]
        public void GetRequest()
        {
            var headers = new Dictionary<string, string>();
            headers["User-Agent"] = "FooAgent";
            headers["Accept"] = "application/json";
            var rid = session.Get("https://httpbin.org/get", headers);

            bool done = false;
            session.RequestComplete += (ref Response res) =>
            {
                done = true;
                TestContext.Out.WriteLine(res.Body.ToStringAlloc());
                
                Assert.AreEqual(rid, res.RequestId, "Request id mismatch");
                Assert.AreEqual(200, res.HttpStatus);

                Assert.IsTrue(res.Headers.HeaderEquals("Content-Type", "application/json"));

                var index = res.Headers.FindHeader("Content-Type");
                Assert.IsTrue(index >= 0);
                Assert.AreEqual("application/json", res.Headers.GetHeaderAlloc(index));

                string headerName;
                string headerValue;
                res.Headers.GetHeaderAlloc(index, out headerName, out headerValue);
                Assert.AreEqual("Content-Type", headerName);
                Assert.AreEqual("application/json", headerValue);

                string headerValue2;
                Assert.IsTrue(res.Headers.TryGetHeaderAlloc("Content-Type", out headerValue2));
                Assert.AreEqual("application/json", headerValue2);

                Assert.IsTrue(res.Body.ToStringAlloc().Contains("origin"));
            };

            for (var i = 0; !done && i < 50; ++i)
            {
                session.Update();
                Thread.Sleep(100);
            }

            Assert.IsTrue(done, "Response never arrived");
        }

        [Test]
        public void PostRequest()
        {
            var headers = new Dictionary<string, string>();
            headers["User-Agent"] = "FooAgent";
            headers["Accept"] = "application/json";
            headers["Content-Type"] = "application/json";
            var postBody = Encoding.UTF8.GetBytes("{\"foo\": \"bar\", \"baz\": 33}");
            var rid = session.Post("https://httpbin.org/post", postBody, postBody.Length, headers);

            bool done = false;
            session.RequestComplete += (ref Response res) =>
            {
                done = true;
                TestContext.Out.WriteLine(res.Body.ToStringAlloc());
                
                Assert.AreEqual(rid, res.RequestId, "Request id mismatch");
                Assert.AreEqual(200, res.HttpStatus);

                Assert.IsTrue(res.Headers.HeaderEquals("Content-Type", "application/json"));

                var index = res.Headers.FindHeader("Content-Type");
                Assert.IsTrue(index >= 0);
                Assert.AreEqual("application/json", res.Headers.GetHeaderAlloc(index));

                string headerName;
                string headerValue;
                res.Headers.GetHeaderAlloc(index, out headerName, out headerValue);
                Assert.AreEqual("Content-Type", headerName);
                Assert.AreEqual("application/json", headerValue);

                string headerValue2;
                Assert.IsTrue(res.Headers.TryGetHeaderAlloc("Content-Type", out headerValue2));
                Assert.AreEqual("application/json", headerValue2);

                Assert.IsTrue(res.Body.ToStringAlloc().Contains("\"baz\""));
            };

            for (var i = 0; !done && i < 50; ++i)
            {
                session.Update();
                Thread.Sleep(100);
            }

            Assert.IsTrue(done, "Response never arrived");
        }
    }
}
